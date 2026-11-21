USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListJobChangeLog]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-19
description : ?? ?? ??? ?????.
result set : JobCode/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103142 = ?? ??? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListJobChangeLog]
	  @inbCharacterID bigint --// ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = @inbCharacterID;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT JobCode
	FROM dbo.JobChangeLogs WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID
	ORDER BY JobChangeLogID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103142;
		GOTO ErrorHandler;
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
