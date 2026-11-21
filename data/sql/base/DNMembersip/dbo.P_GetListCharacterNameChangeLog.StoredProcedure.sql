USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacterNameChangeLog]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListCharacterNameChangeLog]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListCharacterNameChangeLog] AS' 
END
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-11-12
description : ??? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101184 = ??? ?? ?? ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_GetListCharacterNameChangeLog]
	  @inbCharacterID bigint --// ??? ID
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
	/**_# [CharacterNameChangeLogs] ???? SELECT???.*/
	SELECT CharacterNameBefore, CharacterNameAfter, CharacterNameChangeCode, ChangeDate
	FROM dbo.CharacterNameChangeLogs WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101184;
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
