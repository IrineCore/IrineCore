USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelPartyForServer]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-01-05
description : ??? ??? ??? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DelPartyForServer]
	  @intServerID int --// ??ID
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
	/**_# ?? Party? ??? ???? Delete???.*/
	DELETE dbo.PartyJoins
	FROM dbo.Parties P WITH(UPDLOCK)
		INNER JOIN dbo.PartyJoins PJ  ON P.PartyID = PJ.PartyID
	WHERE P.ServerID = @intServerID;
	
	/**_# ?? Party? Delete???.*/
	DELETE dbo.Parties 
	FROM dbo.Parties 
	WHERE ServerID = @intServerID;

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
