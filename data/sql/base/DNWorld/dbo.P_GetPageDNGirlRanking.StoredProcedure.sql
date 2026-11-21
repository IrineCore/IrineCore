USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageDNGirlRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-02
description : ??? ??? ??????.
result set : Ranking/int, CharacterName/nvarchar, FanCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103410 = ?? ???? ?? ????.
**/

CREATE PROCEDURE [dbo].[P_GetPageDNGirlRanking]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.
	, @inbCharacterID bigint = NULL --// ???ID (optional)
	, @inbDNGirlCharacterID bigint = NULL --// ??? ???ID (optional)
	, @intTotalRankingCount int = NULL OUTPUT  --// ?? ?? ???	
	, @intDNGirlRanking int = NULL OUTPUT --// ??? ??
	, @intDNGirlFanCount int = NULL OUTPUT --// ??? ? ?
	, @nvcDNGirlCharacterName nvarchar(30) = NULL OUTPUT --// ??? ????
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
	IF @intPageNumber = 1
		SELECT @intTotalRankingCount = COUNT(*)
		FROM dbo.VIX_DNGirlFanCount WITH(NOLOCK);
	
	/**_# [DNGirlIntimacy] ???? SELECT???.*/
	WITH CTE AS (
		SELECT ROW_NUMBER() OVER(ORDER BY FC.FanCount DESC) AS RowNumber, RANK() OVER(ORDER BY FC.FanCount DESC, FC.TotalFanIntimacy DESC) AS Ranking, CH.CharacterName, FC.FanCount
		FROM dbo.VIX_DNGirlFanCount FC WITH (NOLOCK, NOEXPAND) 
			 INNER JOIN dbo.Characters CH WITH (NOLOCK) ON FC.DNGirlCharacterID = CH.CharacterID
		WHERE NOT EXISTS (SELECT DNGirlCharacterID FROM dbo.BlockedDNGirls WITH (NOLOCK) WHERE FC.DNGirlCharacterID = DNGirlCharacterID)
	)
	SELECT Ranking, CharacterName, FanCount
	FROM CTE 
	WHERE RowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
	ORDER BY Ranking ASC;
	
	/**_# ???? ???? ?? ??? ??? ? ???? ?????.*/
	IF @inbDNGirlCharacterID IS NOT NULL
	BEGIN
		WITH A AS (
			SELECT RANK() OVER(ORDER BY FanCount DESC, TotalFanIntimacy DESC) AS Ranking, DNGirlCharacterID, FanCount
			FROM dbo.VIX_DNGirlFanCount WITH (NOLOCK, NOEXPAND) 
		)
		SELECT @intDNGirlRanking = Ranking, @intDNGirlFanCount = FanCount
		FROM A
		WHERE DNGirlCharacterID = @inbDNGirlCharacterID;
	END
	
	/**_# ???? ??? ?? ??? ?? ? ???? ??? ? ???, ??? ????? ?????.*/
	IF @inbCharacterID IS NOT NULL
	BEGIN		
		SELECT @inbDNGirlCharacterID = DNGirlCharacterID
		FROM dbo.DNGirlIntimacy WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID AND FanFlag = 1;
		
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103410;
			GOTO ErrorHandler;
		END
		
		;WITH A AS (
			SELECT RANK() OVER(ORDER BY FanCount DESC, TotalFanIntimacy DESC) AS Ranking, DNGirlCharacterID, FanCount
			FROM dbo.VIX_DNGirlFanCount WITH (NOLOCK, NOEXPAND) 
		)
		SELECT @intDNGirlRanking = A.Ranking, @intDNGirlFanCount = A.FanCount, @nvcDNGirlCharacterName = CH.CharacterName
		FROM A INNER JOIN dbo.Characters CH WITH (NOLOCK) ON A.DNGirlCharacterID = CH.CharacterID
		WHERE A.DNGirlCharacterID = @inbDNGirlCharacterID;
	END
	
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
