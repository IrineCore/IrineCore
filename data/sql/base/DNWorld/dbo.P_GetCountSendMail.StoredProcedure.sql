USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountSendMail]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-27
description : ?? ?? ??? ?? ?? ??????. (?? ?? ???? ?? ?, ?? ?? ?? ?? ?)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2	?? ??? ??? ????? ??
**/

CREATE PROCEDURE [dbo].[P_GetCountSendMail]
	  @inbSenderCharacterID bigint --// ?? ??? ID
	, @intPremiumMailCount int OUTPUT --// ?? ?? ???? ?? ?
	, @intBasicMailCount int OUTPUT --// ?? ?? ?? ?? ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE();

DECLARE @tblMailCount table (DeliveryTypeCode tinyint NOT NULL, MailCount int NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblMailCount (DeliveryTypeCode, MailCount)
	SELECT DeliveryTypeCode, COUNT(*)
	FROM dbo.Mails WITH (NOLOCK)
	WHERE SenderCharacterID = @inbSenderCharacterID AND SendDate >= CAST(CAST(GETDATE() AS date) AS datetime2(0)) AND DeliveryTypeCode < 3
	GROUP BY DeliveryTypeCode;

	SELECT @intBasicMailCount = MailCount FROM @tblMailCount WHERE DeliveryTypeCode = 1;

	IF @@ROWCOUNT = 0
		SET @intBasicMailCount = 0;

	SELECT @intPremiumMailCount = MailCount FROM @tblMailCount WHERE DeliveryTypeCode = 2;

	IF @@ROWCOUNT = 0
		SET @intPremiumMailCount = 0;

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
