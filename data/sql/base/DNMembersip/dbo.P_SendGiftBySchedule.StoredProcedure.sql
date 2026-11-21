USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_SendGiftBySchedule]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_SendGiftBySchedule]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_SendGiftBySchedule] AS' 
END
GO
/**
version : 22
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-04-29

description :
	????? ?? ??? ?????. (dbo.P_GetCountNotReceivedGift ?? ??)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	6 = ?? ??
	7 = ?? ???(???) ??? ??? ???? ???? ?? ??     by 20110530.wgkim
	8 = ??? '|' ??, @intReturnValue OUTPUT ??     by 20110614.wgkim
	9 = ??? '|' ?? at 2011-06-17
	10 = ??? ',' ??     by 20110922.wgkim
	11~14 = (43051) ItemOption ?? by ??? at 2011-11-01
	15 = ItemOption Split ?? > ??? >= ? ??     by ??? at 2011-12-08
	16 = IX_NN_GiftSendSchedule_GiftSendDate_GiftExpireDate ??     by ??? at 2012-01-26
	17~19 = Ver16 ?? ??     by ??? at 2012-01-26
	20~21 = Tuning     by ??? at 2012-01-26
	22 = Tuning     by ??? at 2012-01-30
**/

ALTER PROCEDURE [dbo].[P_SendGiftBySchedule]
	  @inbCharacterID bigint --// ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dtmNowDate datetime = GETDATE()
	  , @intSeqNo int
	  , @intMaxSeqNo int
	  , @intGiftSendScheduleID int			-- ??????ID
	  , @inyGiftTypeCode tinyint			-- ???? (0=??, 1=?????)
	  , @nvcGiftSenderName nvarchar(50)		-- ??? ?? ??
	  , @intGiftLifespan int				-- ?????? ?? ??. ?? : ?
	  , @vchIP varchar(15)					-- ????? ?? IP
	  , @intProductID int					-- ?? ID
	  , @vchSelectedItemIDs varchar(max)	-- ','? ?????? ??? ID ???
	  , @nvcGiftMemo nvarchar(100)			-- ?? ?? ????? ??? ??? ??
	  , @dt2OrderDate datetime2(0)			-- ????
	  , @inyPayMethodCode tinyint			-- 1=????, 2=??, 3=??, 4=??, 5=?????, 6=??? ??, 7=?? ??, 8=??? ?? ? ??, 9=VIP ??, 10=PVP ?? ???, 11=?? ?? ???, 12=?? ?? ???, 13=?? ?? ???, 14=??2, 15=Time Event ??, 16=??? ?? ???, 17=??? ??
	  , @bitNewFlag bit						-- ??? ??? ??? ?? (0=?? ??, 1=?? ??)
	  , @dt2CreateDate datetime2(0)			-- ??? ???
	  , @vchItemOptions varchar(max)		-- ','? ?????? ??? ?? ???
	  , @inbPurchaseOrderID bigint			-- ??? ?? ?? ID
	  , @inbPurchaseOrderDetailID bigint	-- ?? ?? ?? ID

DECLARE @Temp_GiftSendScheduleID TABLE (SeqNo int identity, GiftSendScheduleID int);

DECLARE @Temp_SelectedItemID TABLE (ItemID int, ItemOption tinyint);

CREATE TABLE #Temp_ItemID (seq int IDENTITY(1, 1) PRIMARY KEY, ItemID int);

CREATE TABLE #Temp_ItemOption (seq int IDENTITY(1, 1) PRIMARY KEY, ItemOption tinyint);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	-- ??? ???
	SELECT @dt2CreateDate = CreateDate
	  FROM dbo.Characters WITH (NOLOCK)
	 WHERE CharacterID = @inbCharacterID;

	-- ???? ? ?? ?? ????
	INSERT @Temp_GiftSendScheduleID (GiftSendScheduleID)
	SELECT GS.GiftSendScheduleID
	  FROM dbo.GiftSendSchedule GS WITH (NOLOCK, INDEX(IX_NN_GiftSendSchedule_GiftTypeCode_GiftSendDate_GiftExpireDate_DeleteFlag), FORCESEEK)
	 WHERE @dtmNowDate >= GS.GiftSendDate
	   AND @dtmNowDate <= GS.GiftExpireDate
	   AND GS.GiftSendDate > @dt2CreateDate
	   AND GS.DeleteFlag = 0
	   AND GS.GiftTypeCode = 0
	   AND NOT EXISTS (SELECT * FROM dbo.GiftSendScheduleReceive GR WITH (NOLOCK) WHERE GR.GiftSendScheduleID = GS.GiftSendScheduleID AND GR.CharacterID = @inbCharacterID)
	UNION ALL
	SELECT GT.GiftSendScheduleID
	  FROM dbo.GiftSendScheduleTarget GT WITH (NOLOCK, INDEX(PK_GiftSendScheduleTarget), FORCESEEK)
		   INNER JOIN dbo.GiftSendSchedule GS WITH (NOLOCK, INDEX(PK_GiftSendSchedule), FORCESEEK) ON (GT.GiftSendScheduleID = GS.GiftSendScheduleID)
	 WHERE GT.CharacterID = @inbCharacterID
	   AND GT.DeleteFlag = 0
	   AND @dtmNowDate >= GS.GiftSendDate
	   AND @dtmNowDate <= GS.GiftExpireDate
	   AND GS.GiftSendDate > @dt2CreateDate
	   AND GS.DeleteFlag = 0
	   AND GS.GiftTypeCode > 0
	   AND NOT EXISTS (SELECT * FROM dbo.GiftSendScheduleReceive GR WITH (NOLOCK) WHERE GR.GiftSendScheduleID = GT.GiftSendScheduleID AND GR.CharacterID = GT.CharacterID)
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

	SELECT @intSeqNo = 1, @intMaxSeqNo = @@ROWCOUNT

	IF @intMaxSeqNo = 0 RETURN 0;

	WHILE (@intSeqNo <= @intMaxSeqNo)
	BEGIN
		DELETE @Temp_SelectedItemID;

		TRUNCATE TABLE #Temp_ItemID;
		TRUNCATE TABLE #Temp_ItemOption;

		-- ?? ??
		SELECT @dt2OrderDate			 = GS.GiftSendDate
			 , @intGiftSendScheduleID	 = TGS.GiftSendScheduleID
			 , @inyGiftTypeCode			 = GS.GiftTypeCode
			 , @nvcGiftSenderName		 = GS.GiftSenderName
			 , @intGiftLifespan			 = GS.GiftLifespan
			 , @vchIP					 = GS.IP
			 , @intProductID			 = GS.ProductID
			 , @vchSelectedItemIDs		 = GS.SelectedItemIDs
			 , @nvcGiftMemo				 = GS.GiftMemo
			 , @inyPayMethodCode		 = GS.PayMethodCode
			 , @bitNewFlag				 = GS.NewFlag
		     , @vchItemOptions			 = GS.ItemOptions
		     , @inbPurchaseOrderID		 = 0
		     , @inbPurchaseOrderDetailID = 0
		  FROM @Temp_GiftSendScheduleID TGS
			   INNER JOIN dbo.GiftSendSchedule GS WITH (NOLOCK) ON (TGS.GiftSendScheduleID = GS.GiftSendScheduleID)
		 WHERE TGS.SeqNo = @intSeqNo;

		IF @vchItemOptions IS NULL
		BEGIN
			INSERT @Temp_SelectedItemID (ItemID)
			SELECT SUBSTRING(',' + @vchSelectedItemIDs + ',', n + 1, CHARINDEX(',', ',' + @vchSelectedItemIDs + ',', n + 1) - n - 1)
			  FROM dbo.Tally WITH (NOLOCK)
			 WHERE n < DATALENGTH(',' + @vchSelectedItemIDs + ',')
			   AND SUBSTRING(',' + @vchSelectedItemIDs + ',', n, 1) = ','
			   AND SUBSTRING(',' + @vchSelectedItemIDs + ',', n + 1, CHARINDEX(',', ',' + @vchSelectedItemIDs + ',', n + 1) - n - 1) > 0
		END
		ELSE
		BEGIN
			INSERT #Temp_ItemID (ItemID)
			SELECT SUBSTRING(',' + @vchSelectedItemIDs + ',', n + 1, CHARINDEX(',', ',' + @vchSelectedItemIDs + ',', n + 1) - n - 1)
			  FROM dbo.Tally WITH (NOLOCK)
			 WHERE n < DATALENGTH(',' + @vchSelectedItemIDs + ',')
			   AND SUBSTRING(',' + @vchSelectedItemIDs + ',', n, 1) = ','
			   AND SUBSTRING(',' + @vchSelectedItemIDs + ',', n + 1, CHARINDEX(',', ',' + @vchSelectedItemIDs + ',', n + 1) - n - 1) > 0

			INSERT #Temp_ItemOption (ItemOption)
			SELECT SUBSTRING(',' + @vchItemOptions + ',', n + 1, CHARINDEX(',', ',' + @vchItemOptions + ',', n + 1) - n - 1)
			  FROM dbo.Tally WITH (NOLOCK)
			 WHERE n < DATALENGTH(',' + @vchItemOptions + ',')
			   AND SUBSTRING(',' + @vchItemOptions + ',', n, 1) = ','
			   AND SUBSTRING(',' + @vchItemOptions + ',', n + 1, CHARINDEX(',', ',' + @vchItemOptions + ',', n + 1) - n - 1) >= 0

			INSERT @Temp_SelectedItemID (ItemID, ItemOption)
			SELECT A.ItemID, B.ItemOption
			  FROM #Temp_ItemID A
				   INNER JOIN #Temp_ItemOption B ON (A.seq = B.seq);
		END

		BEGIN TRANSACTION

			INSERT dbo.PurchaseOrders (CharacterID, ReceiverCharacterID, GiftFlag, PCRoomFlag, Price, IP, OrderDate, OrderStatusCode, PayMethodCode, OrderKey, OrderResult, OrderRollbackResult, GiftReceiveFlag, GiftMemo, GiftExpireDate, GiftSenderName, GiftRejectFlag, NewFlag)
			SELECT NULL, @inbCharacterID, 1, 0, 0, dbo.FN_IP2IPNumber(@vchIP), @dt2OrderDate, 2, @inyPayMethodCode, NULL, NULL, NULL, 0, @nvcGiftMemo, DATEADD(MINUTE, @intGiftLifespan, @dt2OrderDate), @nvcGiftSenderName, 0, @bitNewFlag;

			SET @inbPurchaseOrderID = SCOPE_IDENTITY();
			INSERT dbo.PurchaseOrderDetails (PurchaseOrderID, ProductID, ListPrice)
			SELECT @inbPurchaseOrderID, @intProductID, 0;

			SET @inbPurchaseOrderDetailID = SCOPE_IDENTITY();

			INSERT dbo.SelectedItems (PurchaseOrderDetailID, ItemID, GiveFailFlag, ItemOption)
			SELECT @inbPurchaseOrderDetailID, ItemID, 0, ItemOption
			  FROM @Temp_SelectedItemID;

			INSERT dbo.GiftSendScheduleReceive (GiftSendScheduleID, CharacterID, ReceiveDate)
			SELECT @intGiftSendScheduleID, @inbCharacterID, @dtmNowDate;

		COMMIT TRANSACTION

		SELECT @intSeqNo = @intSeqNo + 1;
	END

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
