USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListCCU]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListCCU]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListCCU] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-03-09
description : ?? ?? ?? ??? ?????.
result set : AccountName/nvarchar, IP/varchar, CharacterID/bigint, CharacterName/nvarchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history : 
??ID ????? ?? by dailykim.20111122
**/

ALTER PROCEDURE [dbo].[P_GetListCCU]
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
	SELECT SS.AccountName, dbo.FN_IPNumber2IP(AC.LastLoginIP) AS IP, CharacterDBID AS CharacterID, CharacterName, SS.WorldID
	FROM dbo.DNAuth SS WITH (NOLOCK)
		INNER JOIN dbo.Accounts AC WITH (NOLOCK) ON SS.AccountDBID = AC.AccountID
	WHERE SS.CertifyingStep = 2 AND SS.CharacterDBID > 0;
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
