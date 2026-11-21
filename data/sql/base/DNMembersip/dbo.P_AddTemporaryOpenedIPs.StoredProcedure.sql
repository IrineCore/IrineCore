USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddTemporaryOpenedIPs]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddTemporaryOpenedIPs]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddTemporaryOpenedIPs] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-05
description : ?? ?? ? ?? ??? IP? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101107 = ?? ?? ??? ???? ????.
history :
2 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

ALTER PROCEDURE [dbo].[P_AddTemporaryOpenedIPs]
	  @intTemporaryBlockID int --// ?? ?? ID
	, @vchIPs int --// |? ?????? ?? ??? IP ?? ???
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
	/**_# [TemporaryOpenedIPs] ???? INSERT???.*/
	INSERT dbo.TemporaryOpenedIPs (TemporaryBlockID, IP)
	SELECT TemporaryBlockID, dbo.FN_IP2IPNumber(string) AS IP
	FROM dbo.TF_Split(@vchIPs, '|') F
		CROSS JOIN dbo.TemporaryBlock TB WITH (NOLOCK)
	WHERE TB.TemporaryBlockID = @intTemporaryBlockID AND TB.DeleteFlag = CAST(0 AS bit) AND F.string > ''
	ORDER BY IP;

	/**_# @intTemporaryBlockID ? ???? ??? ???? ??? ?? ??? ???? 101107? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101107;
		GOTO ErrorHandler;
	END
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_TemporaryBlock_TemporaryOpenedIPs_TemporaryBlockID%'
		SET @intReturnValue = 101107;

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
