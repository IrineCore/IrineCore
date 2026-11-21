USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListEventResult]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-03
description : ??? ?? ??? ??? ?????.
result set : AccountID/int, AccountName/nvarchar, CharacterID/bigint, CharacterName/nvarchar, WorldID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
104104 = ??? ?? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetListEventResult]
	  @intEventID int --// ??? ID
	, @datBasisDate date --// ??? ??
	, @intEventSubID int --// ??? ?? ID
WITH EXECUTE AS 'SPExecutor'
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
	SELECT AccountID, AccountName, CharacterID, CharacterName, WorldID, CharacterClassCode, JobCode, CharacterLevel
	FROM dbo.EventResults WITH (NOLOCK)
	WHERE EventID = @intEventID AND BasisDate = @datBasisDate AND EventSubID = @intEventSubID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 104104;
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
