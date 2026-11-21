USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGiftSendSchedule]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListGiftSendSchedule]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListGiftSendSchedule] AS' 
END
GO
/**
version : 5
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-05-23

description :
	??? ?? ??
		@inyType=1 ? ??, @intGiftSendScheduleID ? ???? ?? ????? ???? @intGiftSendScheduleID ???? ??
		@inyType=2 ? ??, @intGiftSendScheduleID ? ?? ???? / SendDate ? ???? ??? ????? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1 = ?? ??
	3 = ??? ??? ??? ??     by 20110929.wgkim
	4 = ???? ?? ??? ?? ???? ?? (???? ??)     by 20111011.wgkim
	5 = ?? ?? by ??? at 2011-10-31
**/

ALTER PROCEDURE [dbo].[P_GetListGiftSendSchedule]
	  @inyType int --// ???? (1=????, 2=???? ????)
	, @intGiftSendScheduleID int = NULL --// ????ID (@inyType=2 ? ???? ??)
	, @sdtStartDate smalldatetime --// ?? ??? (?? ?? ?? ??)
	, @sdtEndDate smalldatetime --// ?? ??? (?? ?? ?? ??)
	, @nvcGiftSenderName nvarchar(50) = NULL --// ?? ??? ??
	, @intProductID int = NULL --// ??ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	-- ?? ?? ??
	IF @inyType = 1
	BEGIN
		IF @intGiftSendScheduleID IS NOT NULL
		BEGIN
			SELECT GiftSendScheduleID, GiftSendDate, GiftSenderName, ProductID, GiftMemo, GiftExpireDate, DeleteFlag
			  FROM dbo.GiftSendSchedule WITH (NOLOCK)
			 WHERE (GiftSendScheduleID = @intGiftSendScheduleID)
			 ORDER BY GiftSendScheduleID;
		END
		ELSE
		BEGIN
			SELECT GiftSendScheduleID, GiftSendDate, GiftSenderName, ProductID, GiftMemo, GiftExpireDate, DeleteFlag
			  FROM dbo.GiftSendSchedule WITH (NOLOCK)
			 WHERE (GiftSendDate BETWEEN @sdtStartDate AND @sdtEndDate)
			   AND (@nvcGiftSenderName IS NULL OR GiftSenderName = @nvcGiftSenderName)
			   AND (@intProductID IS NULL OR ProductID = @intProductID)
			 ORDER BY GiftSendScheduleID;
		END
	END
	-- ???? ???? ??
	ELSE IF @inyType = 2
	BEGIN
		SELECT C.CharacterName, T.CharacterID, S.ReceiveDate, T.DeleteFlag
		  FROM dbo.GiftSendScheduleTarget T WITH (NOLOCK)
		       INNER JOIN dbo.Characters C WITH (NOLOCK) ON (T.CharacterID = C.CharacterID)
		       LEFT OUTER JOIN dbo.GiftSendScheduleReceive S WITH (NOLOCK) ON (T.GiftSendScheduleID = S.GiftSendScheduleID AND T.CharacterID = S.CharacterID)
		 WHERE (T.GiftSendScheduleID = @intGiftSendScheduleID)
		 ORDER BY C.CharacterName;
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
