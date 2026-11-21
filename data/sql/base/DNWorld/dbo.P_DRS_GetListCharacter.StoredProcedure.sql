USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames.com

created date : 2012-02-01
description :
	??? ?? ??
	- @intWorldID = NULL ? ???? ?? ??? ?? ??? ?? ?? (??? ??? ??)
result set : 
	AccountID / int
	AccountName / nvarchar(50)
	CharacterID / bigint
	CharacterName / nvarchar(30)
	WorldID / int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103102 = ???? ???? ????.
history :
	1~2 = ????
**/

CREATE PROCEDURE [dbo].[P_DRS_GetListCharacter]
	  @intAccountName nvarchar(50) --// ??? ?? ??
	, @intWorldID int = NULL --// ?? ID
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

	/**_# ??? ??? ?? ???. */
	SELECT AccountID, AccountName, CharacterID, CharacterName, WorldID
	  FROM dbo.Characters WITH (NOLOCK)
	 WHERE (DeleteFlag = CONVERT(bit, 0))
	   AND (AccountName = @intAccountName)
	   AND (@intWorldID IS NULL OR WorldID = @intWorldID);

	/**_## ???? ???? ???, 103102 ? ?????.*/
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
