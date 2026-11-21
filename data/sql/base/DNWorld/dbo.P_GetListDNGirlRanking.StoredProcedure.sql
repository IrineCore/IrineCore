USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDNGirlRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-01
description : ??? ?? ???? ?????.(TOP 10)
result set : Ranking/int, CharacterName/nvarchar, FanCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListDNGirlRanking]	  
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
	/**_# [DNGirlIntimacy] ???? SELECT???.*/
	SELECT TOP 10 RANK() OVER(ORDER BY FC.FanCount DESC, FC.TotalFanIntimacy DESC) AS Ranking, CH.CharacterName, FC.FanCount
	FROM dbo.VIX_DNGirlFanCount FC WITH (NOLOCK, NOEXPAND) 
		 INNER JOIN dbo.Characters CH WITH (NOLOCK) ON FC.DNGirlCharacterID = CH.CharacterID
    WHERE NOT EXISTS (SELECT DNGirlCharacterID FROM dbo.BlockedDNGirls WITH (NOLOCK) WHERE FC.DNGirlCharacterID = DNGirlCharacterID);
	
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
