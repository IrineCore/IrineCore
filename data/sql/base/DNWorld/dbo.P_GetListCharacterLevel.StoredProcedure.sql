USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCharacterLevel]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-15
description : ?? ??? ?? ???? ?? ??? ?????.
result set : CharacterID/bigint, CharacterLevel/tinyint
return value : 
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
2 = input param. @intWorldID ?? at 2010-10-21
**/

CREATE PROCEDURE [dbo].[P_GetListCharacterLevel]
	  @intAccountID int --// ??? ?? ID
	, @intWorldID int = NULL --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @intWorldID IS NULL
		SELECT @intWorldID = WorldID FROM dbo.WorldCoverage;

	/**_# ??? ??? ?????.*/
	SELECT CharacterID, CharacterLevel
	FROM dbo.V_Characters WITH (NOLOCK)
	WHERE AccountID = @intAccountID AND WorldID = @intWorldID AND DeleteFlag = CAST(0 AS bit);

	/**_# ???? ???? ??? 103102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
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
