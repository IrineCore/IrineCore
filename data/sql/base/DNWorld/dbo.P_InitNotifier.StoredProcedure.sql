USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_InitNotifier]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-14
description : ??? ??? ??? ??????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_InitNotifier]
	  @inbCharacterID bigint --// ??? ID
	, @intNotifierIndexSize tinyint --// ??? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblNotifiers table (NotifierIndex tinyint NOT NULL PRIMARY KEY, NotifyTargetCode tinyint NOT NULL, NotifyTargetID int NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	DELETE dbo.Notifiers WHERE CharacterID = @inbCharacterID;

	INSERT dbo.Notifiers (CharacterID, NotifierIndex, NotifyTargetCode, NotifyTargetID)
	SELECT @inbCharacterID, n - 1, 0, 0
	FROM dbo.Tally WITH (NOLOCK)
	WHERE n BETWEEN 1 AND @intNotifierIndexSize;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_Notifiers_CharacterID%'
		SET @intReturnValue = 103102;

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
