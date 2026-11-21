USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountReceiveMail]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-27
description : ?? ?? ??? ?? ?? ??????. (? ?? ?? ?, ?? ?? ?? ?, ?? ??? 7? ?? ?? ?)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
6 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
7~8 = ??? ?????? ?? ?? by ??? at 2013-04-03
**/

CREATE PROCEDURE [dbo].[P_GetCountReceiveMail]
	  @inbReceiverCharacterID bigint --// ?? ??? ID
	, @intTotalMailCount int OUTPUT --// ? ?? ?? ?
	, @intNotReadMailCount int OUTPUT --// ?? ?? ?? ?
	, @int7DaysLeftMailCount int OUTPUT --// ?? ??? 7? ?? ?? ?
	, @inyMailTabCode tinyint = NULL --// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE();

DECLARE @tblMailCount table (ReadFlag bit NOT NULL, MailCount int NOT NULL, SevenDaysLeftFlag bit NOT NULL);
DECLARE @tblMailTypeCode table (MailTypeCode tinyint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

IF @inyMailTabCode IS NULL
	INSERT @tblMailCount (ReadFlag, MailCount, SevenDaysLeftFlag)
	SELECT ReadFlag, COUNT(*), CASE WHEN DATEDIFF(DAY, SendDate, @dt2Now) >= 23 THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END
	FROM dbo.Mails WITH (NOLOCK, INDEX(IX_NNI_Mails_ReceiverCharacterID), FORCESEEK)	 		 
	WHERE ReceiverCharacterID = @inbReceiverCharacterID AND DeleteFlag = CAST(0 AS bit) AND SendDate >= CAST(CAST(DATEADD(DAY, -29, @dt2Now) AS date) AS datetime2(0))	  	  		  
	GROUP BY ReadFlag, CASE WHEN DATEDIFF(DAY, SendDate, @dt2Now) >= 23 THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END
	OPTION (KEEPFIXED PLAN, MAXDOP 1);
ELSE
	INSERT @tblMailCount (ReadFlag, MailCount, SevenDaysLeftFlag)
	SELECT ReadFlag, COUNT(*), CASE WHEN DATEDIFF(DAY, SendDate, @dt2Now) >= 23 THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END
	FROM dbo.Mails WITH (NOLOCK, INDEX(IX_NNI_Mails_ReceiverCharacterID), FORCESEEK)	 		 
	WHERE ReceiverCharacterID = @inbReceiverCharacterID AND DeleteFlag = CAST(0 AS bit) AND SendDate >= CAST(CAST(DATEADD(DAY, -29, @dt2Now) AS date) AS datetime2(0))	  	  
		  AND MailTabCode = @inyMailTabCode
	GROUP BY ReadFlag, CASE WHEN DATEDIFF(DAY, SendDate, @dt2Now) >= 23 THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

SELECT @intTotalMailCount = ISNULL(SUM(MailCount), 0)
FROM @tblMailCount
OPTION (KEEPFIXED PLAN);

IF @@ROWCOUNT = 0
BEGIN
	SET @intTotalMailCount = 0;
	SET @intNotReadMailCount = 0;
	SET @int7DaysLeftMailCount = 0;
END

ELSE
BEGIN
	SELECT @intNotReadMailCount = ISNULL(SUM(MailCount), 0)
	FROM @tblMailCount
	WHERE ReadFlag = CAST(0 AS bit)
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intNotReadMailCount = 0;
		SET @int7DaysLeftMailCount = 0;
	END

	ELSE
	BEGIN
		SELECT @int7DaysLeftMailCount = MailCount
		FROM @tblMailCount
		WHERE ReadFlag = CAST(0 AS bit) AND SevenDaysLeftFlag = CAST(1 AS bit)
		OPTION (KEEPFIXED PLAN);

		IF @@ROWCOUNT = 0
			SET @int7DaysLeftMailCount = 0;
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
