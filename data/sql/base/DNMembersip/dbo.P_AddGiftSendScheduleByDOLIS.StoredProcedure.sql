USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGiftSendScheduleByDOLIS]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddGiftSendScheduleByDOLIS]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddGiftSendScheduleByDOLIS] AS' 
END
GO
/**
version : 16
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-09-20

description :
	?? ??? ?????. (???)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101172 = ?? ??? ??? ???? ????.
	103315 = ItemID ? ItemOption ? ??? ???? ????.
history :
	6 = ?? ??
	7 = ??? ','? ??     by 20110926.wgkim
	8 = DeleteFlag ??     by 20110929.wgkim
	9 = DISTINCT ??     by 20111006.wgkim
	10 = TF_NSplit()? IF_NSplit(), IF_Split()?? ?? by ??? at 2011-10-25
	11~14 = (43051) ItemOption ?? by ??? at 2011-10-28
**/

ALTER PROCEDURE [dbo].[P_AddGiftSendScheduleByDOLIS]
	  @inyGiftTypeCode tinyint --// ???? (0=??, 1=??? ??, 2=?? ??)
	, @vchCharacterIDs varchar(max) = NULL --// ?? ?? ???ID ??? (??? : ',') (@inyGiftTypeCode=1 ? ???? ??)
	, @nvcCharacterNames nvarchar(max) = NULL --// ?? ?? ???? ??? (??? : ',') (@inyGiftTypeCode=1 ? ???? ??)
	, @intWorldID int = NULL --// ?? ?? ??ID (@inyGiftTypeCode=2 ? ???? ??)
	, @nvcGiftSenderName nvarchar(50) --// ??? ?? ??
	, @intGiftLifespan int --// ?????? ?? ?? (?? : ?)
	, @vchIP varchar(15) --// ????? ?? IP
	, @intProductID int --// ?? ID
	, @vchSelectedItemIDs varchar(max) --// ??? ID ??? (??? : ',')
	, @nvcGiftMemo nvarchar(100) --// ?? ?? ????? ??? ??? ??
	, @bitNewFlag bit = 0 --// ??? ??? ??? ??
	, @vchItemOptions varchar(max) = NULL --// ??? ?? ??? (??? : ',')
	, @dtmReserveSendDate datetime = NULL --// ???? ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
	  , @intGiftSendScheduleID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	-- ???ID ? ?????? ?? ???? ???? ??? ??
	IF @vchItemOptions IS NOT NULL
	BEGIN
		IF (LEN(@vchSelectedItemIDs) - LEN(REPLACE(@vchSelectedItemIDs, ',', ''))) <> (LEN(@vchItemOptions) - LEN(REPLACE(@vchItemOptions, ',', '')))
		BEGIN
			SET @intReturnValue = 103315;
			GOTO ErrorHandler;
		END
	END

	-- ?? ?? ??
	INSERT dbo.GiftSendSchedule (GiftExpireDate, GiftSendDate, GiftTypeCode, GiftSenderName, GiftLifespan, IP, ProductID, SelectedItemIDs, GiftMemo, PayMethodCode, NewFlag, DeleteFlag, ItemOptions)
	SELECT DATEADD(MI, @intGiftLifespan, COALESCE(@dtmReserveSendDate, @dt2Now)), COALESCE(@dtmReserveSendDate, @dt2Now), @inyGiftTypeCode, @nvcGiftSenderName, @intGiftLifespan, @vchIP, @intProductID, @vchSelectedItemIDs, @nvcGiftMemo, 5, @bitNewFlag, 0, @vchItemOptions;

	SET @intGiftSendScheduleID = SCOPE_IDENTITY();

	-- ?? ?? ?? ?? : ??? ??
	IF @inyGiftTypeCode = 1 AND @intGiftSendScheduleID IS NOT NULL
	BEGIN
		-- ???ID ??
		IF @vchCharacterIDs IS NOT NULL
		BEGIN
			INSERT dbo.GiftSendScheduleTarget (GiftSendScheduleID, CharacterID, DeleteFlag)
			SELECT @intGiftSendScheduleID, SUBSTRING(',' + @vchCharacterIDs + ',', n + 1, CHARINDEX(',', ',' + @vchCharacterIDs + ',', n + 1) - n - 1), 0
			  FROM dbo.Tally WITH (NOLOCK)
			 WHERE n < DATALENGTH(',' + @vchCharacterIDs + ',')
			   AND SUBSTRING(',' + @vchCharacterIDs + ',', n, 1) = ','
			   AND SUBSTRING(',' + @vchCharacterIDs + ',', n + 1, CHARINDEX(',', ',' + @vchCharacterIDs + ',', n + 1) - n - 1) > 0
			 GROUP BY SUBSTRING(',' + @vchCharacterIDs + ',', n + 1, CHARINDEX(',', ',' + @vchCharacterIDs + ',', n + 1) - n - 1);

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 101172;
				GOTO ErrorHandler;
			END
		END
		-- ???? ??
		ELSE IF @vchCharacterIDs IS NULL AND @nvcCharacterNames IS NOT NULL
		BEGIN
			;WITH T1 AS (
				SELECT SUBSTRING(N',' + @nvcCharacterNames + N',', n + 1, CHARINDEX(N',', N',' + @nvcCharacterNames + N',', n + 1) - n - 1) AS CharacterName
				  FROM dbo.Tally WITH (NOLOCK, INDEX(PK_Tally), FORCESEEK)
				 WHERE n < (DATALENGTH(N',' + @nvcCharacterNames + N',') / 2)
				   AND SUBSTRING(N',' + @nvcCharacterNames + N',', n, 1) = N','
				   AND SUBSTRING(N',' + @nvcCharacterNames + N',', n + 1, CHARINDEX(N',', N',' + @nvcCharacterNames + N',', n + 1) - n - 1) > N''
				 GROUP BY SUBSTRING(N',' + @nvcCharacterNames + N',', n + 1, CHARINDEX(N',', N',' + @nvcCharacterNames + N',', n + 1) - n - 1)
			)
			INSERT dbo.GiftSendScheduleTarget (GiftSendScheduleID, CharacterID, DeleteFlag)
			SELECT @intGiftSendScheduleID, C.CharacterID, 0
			  FROM T1
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (CONVERT(nvarchar(30), T1.CharacterName) = C.CharacterName);

			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 101172;
				GOTO ErrorHandler;
			END
		END
		ELSE IF @vchCharacterIDs IS NULL AND @nvcCharacterNames IS NULL
		BEGIN
			SET @intReturnValue = 101172;
			GOTO ErrorHandler;
		END
	END
	-- ?? ?? ?? ?? : ?? ??
	ELSE IF @inyGiftTypeCode = 2 AND @intGiftSendScheduleID IS NOT NULL
	BEGIN
		INSERT dbo.GiftSendScheduleTarget (GiftSendScheduleID, CharacterID, DeleteFlag)
		SELECT @intGiftSendScheduleID, CharacterID, 0
		  FROM dbo.Characters WITH (NOLOCK)
		 WHERE WorldID = @intWorldID
		   AND DeleteFlag = 0;

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 101172;
			GOTO ErrorHandler;
		END
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
