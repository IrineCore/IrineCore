USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMailSchedules]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-05-23

description :
	???? ??
		@inyType=1 ? ??, @intMailScheduleID ? ???? ?? ????? ???? @intMailScheduleID ???? ??
		@inyType=2,3 ? ??, @intMailScheduleID ? ?? ????
		@inyType=3 ? ??, SendDate ? ???? ??? ????? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	4 = ?? ??
	5 = ??? ??? ??? ??     by 20110929.wgkim
	6~9 = ???? ?? ??? ?? ???? ?? (???? ??)     by 20111011.wgkim
**/

CREATE PROCEDURE [dbo].[P_GetListMailSchedules]
	  @inyType int --// ???? (1=????, 2=???? ?????, 3=???? ????)
	, @intMailScheduleID int = NULL --// ?? ?? ID (@inyType=2,3 ? ???? ??)
	, @sdtStartDate smalldatetime --// ?? ??? (?? ?? ?? ??)
	, @sdtEndDate smalldatetime --// ?? ??? (?? ?? ?? ??)
	, @nvcSenderName nvarchar(50) = NULL --// ?? ??? ??
	, @nvcSubject nvarchar(50) = NULL --// ??
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
		IF @intMailScheduleID IS NOT NULL
		BEGIN
			SELECT MailScheduleID, SendDate, SenderName, [Subject], Content, AttachCoin, MailScheduleExpireDate, MailTypeCode, DeleteFlag
			  FROM dbo.MailSchedules WITH (NOLOCK)
			 WHERE (MailScheduleID = @intMailScheduleID)
			 ORDER BY MailScheduleID;
		END
		ELSE
		BEGIN
			SELECT MailScheduleID, SendDate, SenderName, [Subject], Content, AttachCoin, MailScheduleExpireDate, MailTypeCode, DeleteFlag
			  FROM dbo.MailSchedules WITH (NOLOCK)
			 WHERE (SendDate BETWEEN @sdtStartDate AND @sdtEndDate)
			   AND (@nvcSenderName IS NULL OR SenderName = @nvcSenderName)
			   AND (@nvcSubject IS NULL OR [Subject] LIKE N'%' + @nvcSubject + N'%')
			 ORDER BY MailScheduleID;
		END
	END
	-- ?? ?? ?? ??? ??
	ELSE IF @inyType = 2
	BEGIN
		SELECT ItemID, ItemCount, ItemLevel, ItemPotential, ItemOption, EternityFlag, ItemLifespan
		  FROM dbo.AttachItemsForMailSchedule WITH (NOLOCK)
		 WHERE (MailScheduleID = @intMailScheduleID)
		 ORDER BY ItemID;
	END
	-- ?? ?? ?? ?? ??
	ELSE IF @inyType = 3
	BEGIN
		SELECT C.CharacterName, T.CharacterID, S.SendDate, T.DeleteFlag
		  FROM dbo.MailSchedulesTarget T WITH (NOLOCK)
		       INNER JOIN dbo.Characters C WITH (NOLOCK) ON (T.CharacterID = C.CharacterID)
		       LEFT OUTER JOIN dbo.SendedMails S WITH (NOLOCK) ON (T.MailScheduleID = S.MailScheduleID AND T.CharacterID = S.CharacterID)
		 WHERE (T.MailScheduleID = @intMailScheduleID)
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
