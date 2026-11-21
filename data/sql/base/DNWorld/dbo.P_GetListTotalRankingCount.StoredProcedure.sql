USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListTotalRankingCount]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-08-30
description : ??? ?? ?? ??? ?????.
result set : 
	@inyRankingCode = 1 : TotalCount/bigint
	@inyRankingCode = 2 : PVPLadderCode/tinyint, TotalCount/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListTotalRankingCount]
	  @inyRankingCode tinyint --// 1=PVP, 2=PVP??
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
	IF @inyRankingCode = 1
		SELECT TotalCount
		FROM dbo.PVPRankingCount WITH(NOLOCK)
		WHERE GuildName IS NULL
		  AND Total = 1
	ELSE IF @inyRankingCode = 2
		SELECT PVPLadderCode, TotalCount
		FROM dbo.PVPLadderRankingCount WITH(NOLOCK)
		WHERE GuildName IS NULL
		  AND Total = 1
END TRY
BEGIN CATCH
	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
