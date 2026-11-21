USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPVPLadderCUCount]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-02-21
description : PVP ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddPVPLadderCUCount]
	  @inyPVPLadderCode tinyint --// 1=1:1, 2=2:2, 3=3:3, 4=4:4
	, @intCUCount int --// ?? ??? ?
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
	/**_# [PVPLadderCuCountLogs] ???? INSERT???.*/
	INSERT dbo.PVPLadderCuCountLogs (BasisDate, PVPLadderCode, CUCount)
	VALUES (GETDATE(), @inyPVPLadderCode, @intCUCount);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_PVPLadderCuCountLogs%'
		SET @intReturnValue = 100;

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
