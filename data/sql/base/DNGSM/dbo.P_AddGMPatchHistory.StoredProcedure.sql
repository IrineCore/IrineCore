USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGMPatchHistory]    Script Date: 11/14/2026 8:47:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO

/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-12
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
102108 = ?? ??? ?? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddGMPatchHistory]
	  @nvcPatchVersion nvarchar(30) --// ?? ??
	, @dtmPatchDate datetime --// ?? ??
	, @intGMID int --// ??? ID
	, @nvcPatchMemo nvarchar(200) --// ?? ??
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
	/**_# [GMPatchHistory] ???? INSERT???.*/
	INSERT dbo.GMPatchHistory (PatchVersion, PatchDate, GMID, PatchMemo)
	VALUES (@nvcPatchVersion, @dtmPatchDate, @intGMID, @nvcPatchMemo);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_GMPatchHistory%'
		SET @intReturnValue = 102108;

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
