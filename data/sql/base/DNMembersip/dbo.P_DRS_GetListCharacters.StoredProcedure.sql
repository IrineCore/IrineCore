USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetListCharacters]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_GetListCharacters]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_GetListCharacters] AS' 
END
GO
/**
version : 4
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-06-29
description : ??? ??? ???? ?????. (??????)
result set : WorldID/int, CharacterID/bigint, CharacterName/nvarchar, CreateDate/datetime, PetalBalance/int, IsRestraint/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101117 = ???? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_DRS_GetListCharacters]
	  @intAccountID int --// ??? ?? ID	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @intIncomeAmount int = 0
	, @intOutgoAmount int = 0
	, @intPetalBalance int = 0;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ??? ?????.*/
	SELECT @intIncomeAmount = PetalAmount
	FROM dbo.VIX_PetalIncome WITH (NOEXPAND, NOLOCK)
	WHERE AccountID = @intAccountID;

	IF @@ROWCOUNT = 0
		SET @intPetalBalance = 0;

	ELSE
	BEGIN
		SELECT @intOutgoAmount = PetalAmount
		FROM dbo.VIX_PetalOutgo WITH (NOEXPAND, NOLOCK)
		WHERE AccountID = @intAccountID;

		SET @intPetalBalance = @intIncomeAmount - @intOutgoAmount;
	END

	/**_# [Characters] ???? SELECT???.*/
	SELECT C.WorldID, C.CharacterID, C.CharacterName, C.CreateDate, @intPetalBalance AS PetalBalance
		 , CASE WHEN R.AccountID IS NOT NULL THEN CAST(1 AS bit) ELSE CAST(0 AS bit) END AS IsRestraint 
	FROM dbo.Characters C WITH (NOLOCK) LEFT OUTER JOIN dbo.Restraints R WITH (NOLOCK) 
	ON C.AccountID = R.AccountID AND (R.StartDate < GETDATE() AND R.EndDate > GETDATE()) AND CancelFlag = 0
	WHERE C.AccountID = @intAccountID AND DeleteFlag = 0;

	/**_# ??? ???? ?? ??? 101117? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101117;
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
