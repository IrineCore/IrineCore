USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGameMoneyMonitor3]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-10
description : ??? ? ???? ??? TOP 500? ?????.
result set : WorldID/int, AccountName/nvarchar, CharacterID/bigint, CharacterName/nvarchar, GameMoneyAmount/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListGameMoneyMonitor3]
	  @datBasisDate date --// ?? ??
	, @inyCoinChangeCode tinyint --// 1=??? ??, 2=?? ??, 3=??, 4=??? ??, 7=??????, 10=??? ???, 11=?? ??, 23=???? ??? ???, 27=?? ??? ??
    , @bitExcludeRestraintAccount bit = NULL --// ???? ?? ?? (0=??, 1=???)
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
	IF @bitExcludeRestraintAccount = 1
		SELECT DC.WorldID, DA.AccountName, DC.CharacterID, DC.CharacterName, GMM.GameMoneyAmount
		FROM dbo.Mon_GameMoney3 GMM WITH (NOLOCK)
			INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON GMM.CharacterID = DC.CharacterID
			INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
			LEFT OUTER JOIN dbo.SS_RestraintedAccounts R ON DA.AccountID = R.AccountID
		WHERE GMM.BasisDate = @datBasisDate AND GMM.CoinChangeCode = @inyCoinChangeCode AND R.AccountID IS NULL
		ORDER BY GMM.GameMoneyAmount DESC;
	ELSE
		SELECT DC.WorldID, DA.AccountName, DC.CharacterID, DC.CharacterName, GMM.GameMoneyAmount
		FROM dbo.Mon_GameMoney3 GMM WITH (NOLOCK)
			INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON GMM.CharacterID = DC.CharacterID
			INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON DC.AccountID = DA.AccountID
		WHERE GMM.BasisDate = @datBasisDate AND GMM.CoinChangeCode = @inyCoinChangeCode
		ORDER BY GMM.GameMoneyAmount DESC;
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
