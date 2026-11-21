USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGiftSendSchedule]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddGiftSendSchedule]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddGiftSendSchedule] AS' 
END
GO
/**
version : 8
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-05-03

description :
	?? ??? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101172 = ?? ??? ??? ???? ????.
history :
	2 = ?? ??
	3 = ???? ',' ?? '|'? ??     by 20110621.wgkim
	4~5 = DeleteFlag ??     by 20110929.wgkim
	6 = DISTINCT ??     by 20111006.wgkim
	7 = TF_NSplit()? IF_Split()?? ?? by ??? at 2011-10-25
	8 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

ALTER PROCEDURE [dbo].[P_AddGiftSendSchedule]
	  @inyGiftTypeCode tinyint --// ???? (0=??, 1=?????)
	, @vchCharacterIDs varchar(max) = null --// ?? ?? ??? ID ??? (??? : '|') (@inyGiftTypeCode=1 ? ???? ??)
	, @nvcGiftSenderName nvarchar(50) --// ??? ?? ??
	, @intGiftLifespan int --// ?????? ?? ?? (?? : ?)
	, @vchIP varchar(15) --// ????? ?? IP
	, @intProductID int --// ?? ID
	, @vchSelectedItemIDs varchar(max) --// '|'? ?????? ??? ID ???
	, @nvcGiftMemo nvarchar(100) --// ?? ?? ????? ??? ??? ??
	, @inyPayMethodCode tinyint --// 1=????, 2=??, 3=??, 4=??, 5=?????, 6=??? ??, 7=?? ??, 8=??? ?? ? ??, 9=VIP ??, 10=PVP ?? ???, 11=?? ?? ???, 12=?? ?? ???, 13=?? ?? ???, 14=??2, 15=Time Event ??, 16=??? ?? ???, 17=??? ??
	, @bitNewFlag bit = 0 --// ??? ??? ??? ??
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

	INSERT dbo.GiftSendSchedule (GiftExpireDate, GiftSendDate, GiftTypeCode, GiftSenderName, GiftLifespan, IP, ProductID, SelectedItemIDs, GiftMemo, PayMethodCode, NewFlag, DeleteFlag)
	SELECT DATEADD(MI, @intGiftLifespan, @dt2Now), @dt2Now, @inyGiftTypeCode, @nvcGiftSenderName, @intGiftLifespan, @vchIP, @intProductID, @vchSelectedItemIDs, @nvcGiftMemo, @inyPayMethodCode, @bitNewFlag, 0

	SET @intGiftSendScheduleID = SCOPE_IDENTITY();

	IF @inyGiftTypeCode = 1 AND @intGiftSendScheduleID IS NOT NULL
	BEGIN

		INSERT dbo.GiftSendScheduleTarget (GiftSendScheduleID, CharacterID, DeleteFlag)
		SELECT DISTINCT @intGiftSendScheduleID, CONVERT(bigint, string), 0
		  FROM dbo.TF_Split(@vchCharacterIDs, '|');

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
