USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListActiveGameMoney]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-09
description : ?? ?? ?? ?? ??? ?????.
result set : BasisDate, WorldName, TotalGameMoneyAmount, TotalCharacterCount, GameMoneyAmount1, CharacterCount1, GameMoneyAmount2, CharacterCount2
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListActiveGameMoney]
	  @datStartDate date --// ?? ?? ??
	, @datEndDate date --// ?? ? ??
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
	SELECT ST.BasisDate, W.WorldName
		, ISNULL(MAX(CASE ST.ActiveGameMoneyCode WHEN 1 THEN GameMoneyAmount END), 0) AS TotalGameMoneyAmount
		, ISNULL(MAX(CASE ST.ActiveGameMoneyCode WHEN 1 THEN CharacterCount END), 0) AS TotalCharacterCount
		, ISNULL(MAX(CASE ST.ActiveGameMoneyCode WHEN 2 THEN GameMoneyAmount END), 0) AS GameMoneyAmount1
		, ISNULL(MAX(CASE ST.ActiveGameMoneyCode WHEN 2 THEN CharacterCount END), 0) AS CharacterCount1
		, ISNULL(MAX(CASE ST.ActiveGameMoneyCode WHEN 3 THEN GameMoneyAmount END), 0) AS GameMoneyAmount2
		, ISNULL(MAX(CASE ST.ActiveGameMoneyCode WHEN 3 THEN CharacterCount END), 0) AS CharacterCount2
	FROM dbo.ST_ActiveGameMoney ST WITH (NOLOCK)
		INNER JOIN dbo.DimWorlds W WITH (NOLOCK) ON ST.WorldID = W.WorldID
	WHERE ST.BasisDate BETWEEN @datStartDate AND @datEndDate
	GROUP BY ST.BasisDate, ST.WorldID, W.WorldName
	ORDER BY ST.BasisDate, ST.WorldID, W.WorldName;
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
