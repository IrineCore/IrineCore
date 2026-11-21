USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddTemporaryBlock]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddTemporaryBlock]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddTemporaryBlock] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-04
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

ALTER PROCEDURE [dbo].[P_AddTemporaryBlock]
	  @sdtStartDate smalldatetime --// ?? ?? ??
	, @sdtEndDate smalldatetime --// ?? ? ??
	, @intAdminID int --// ??? ID
	, @vchOpenIPs varchar(8000) --// ?? ?? ? ??? IP? |? ??? ???
	, @intTemporaryBlockID int = NULL OUTPUT --// ?? ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @sdtNow smalldatetime = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# DeleteFlag = 0? ???? ?? 1?? ??? ??? DeleteFlag = 1? ???? ?? UPDATE ???.*/
	UPDATE dbo.TemporaryBlock
	SET DeleteFlag = 1, DeleteDate = @sdtNow
	WHERE DeleteFlag = CAST(0 AS bit);

	/**_# [TemporaryBlock] ???? INSERT???.*/
	INSERT dbo.TemporaryBlock (StartDate, EndDate, AdminID, RegisterDate, DeleteFlag, DeleteDate)
	VALUES (@sdtStartDate, @sdtEndDate, @intAdminID, @sdtNow, 0, NULL);

	SET @intTemporaryBlockID = SCOPE_IDENTITY();

	/**_# [TemporaryOpenedIPs] ???? INSERT???.*/
	IF @vchOpenIPs IS NOT NULL
		INSERT dbo.TemporaryOpenedIPs (TemporaryBlockID, IP)
		SELECT @intTemporaryBlockID, dbo.FN_IP2IPNumber(string)
		FROM dbo.TF_Split(@vchOpenIPs, '|')
		WHERE string > ''
		OPTION (KEEPFIXED PLAN);
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
