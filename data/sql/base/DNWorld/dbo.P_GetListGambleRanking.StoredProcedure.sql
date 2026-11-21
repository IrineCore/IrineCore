USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGambleRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-24
description : ???? ??? ?? ?? ??? ?????.
result set : 
	 Ranking, PlayCount, PlayCount_AccountID, PlayCount_AccountName, PlayCount_CharacterID, PlayCount_CharacterName
			, WinCount, WinCount_AccountID, WinCount_AccountName, WinCount_CharacterID, WinCount_CharacterName
			, SemiWinCount, SemiWinCount_AccountID, SemiWinCount_AccountName, SemiWinCount_CharacterID, SemiWinCount_CharacterName

return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListGambleRanking]
	  @inyGambleFeeCode tinyint --// 1=??, 2=??
	, @dt2GambelStartDate datetime2(0) --// ??????
	, @dt2GambelEndDate datetime2(0) --// ??????
	, @intRankCount int --// ????? ?? ?? ?? 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE @tblPlayCountRank TABLE
(
	  Ranking int
	, AccountID int
	, AccountName nvarchar(50)
	, CharacterID bigint
	, CharacterName nvarchar(50)
	, PlayCount int
)
DECLARE @tblWinCountRank TABLE
(
	  Ranking int
	, AccountID int
	, AccountName nvarchar(50)
	, CharacterID bigint
	, CharacterName nvarchar(50)
	, WinCount int
)
DECLARE @tblSemiWinCountRank TABLE
(
	  Ranking int
	, AccountID int
	, AccountName nvarchar(50)
	, CharacterID bigint
	, CharacterName nvarchar(50)
	, SemiWinCount int   
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	;WITH CTE AS
	(
		SELECT B.CharacterID, COUNT(*) AS PlayCount
		FROM dbo.GambleIDs A WITH(NOLOCK, INDEX(IX_NC_GambleIDs_GambelStartDate), FORCESEEK)
			INNER JOIN dbo.GambleParticipantes B WITH(NOLOCK) ON A.GambleID = B.GambleID
		WHERE GambelStartDate BETWEEN @dt2GambelStartDate AND @dt2GambelEndDate
		  AND RewardFlag = 0
		  AND GambleFeeCode = @inyGambleFeeCode
		  AND GambelEndDate IS NOT NULL
		GROUP BY B.CharacterID
	), CTE2 AS
	(
		SELECT ROW_NUMBER() OVER(ORDER BY PlayCount DESC) AS Ranking, B.AccountID, B.AccountName, B.CharacterID, B.CharacterName, A.PlayCount  
		FROM CTE A
			INNER JOIN dbo.Characters B WITH(NOLOCK) ON A.CharacterID = B.CharacterID
	)
	INSERT @tblPlayCountRank (Ranking, AccountID, AccountName, CharacterID, CharacterName, PlayCount)
	SELECT Ranking, AccountID, AccountName, CharacterID, CharacterName, PlayCount   
	FROM CTE2
	WHERE Ranking <= @intRankCount; 
	

	WITH CTE AS
	(
		SELECT WinCharacterID AS CharacterID, COUNT(*) AS WinCount
		FROM dbo.GambleIDs A WITH(NOLOCK, INDEX(IX_NC_GambleIDs_GambelStartDate), FORCESEEK)
		WHERE GambelStartDate BETWEEN @dt2GambelStartDate AND @dt2GambelEndDate
		  AND RewardFlag = 0
		  AND GambleFeeCode = @inyGambleFeeCode
		  AND GambelEndDate IS NOT NULL
		GROUP BY WinCharacterID
	), CTE2 AS
	(
		SELECT ROW_NUMBER() OVER(ORDER BY WinCount DESC) AS Ranking, B.AccountID, B.AccountName, B.CharacterID, B.CharacterName, A.WinCount  
		FROM CTE A
			INNER JOIN dbo.Characters B WITH(NOLOCK) ON A.CharacterID = B.CharacterID
	)
	INSERT @tblWinCountRank (Ranking, AccountID, AccountName, CharacterID, CharacterName, WinCount)
	SELECT Ranking, AccountID, AccountName, CharacterID, CharacterName, WinCount   
	FROM CTE2
	WHERE Ranking <= @intRankCount; 

	WITH CTE AS
	(
		SELECT SemiWinCharacterID AS CharacterID, COUNT(*) AS SemiWinCount
		FROM dbo.GambleIDs A WITH(NOLOCK, INDEX(IX_NC_GambleIDs_GambelStartDate), FORCESEEK)
		WHERE GambelStartDate BETWEEN @dt2GambelStartDate AND @dt2GambelEndDate
		  AND RewardFlag = 0
		  AND GambleFeeCode = @inyGambleFeeCode
		  AND GambelEndDate IS NOT NULL
		GROUP BY SemiWinCharacterID
	), CTE2 AS
	(
		SELECT ROW_NUMBER() OVER(ORDER BY SemiWinCount DESC) AS Ranking, B.AccountID, B.AccountName, B.CharacterID, B.CharacterName, A.SemiWinCount  
		FROM CTE A
			INNER JOIN dbo.Characters B WITH(NOLOCK) ON A.CharacterID = B.CharacterID
	)
	INSERT @tblSemiWinCountRank (Ranking, AccountID, AccountName, CharacterID, CharacterName, SemiWinCount)
	SELECT Ranking, AccountID, AccountName, CharacterID, CharacterName, SemiWinCount   
	FROM CTE2
	WHERE Ranking <= @intRankCount; 

	SELECT A.Ranking, A.PlayCount, A.AccountID AS PlayCount_AccountID, A.AccountName AS PlayCount_AccountName, A.CharacterID AS PlayCount_CharacterID, A.CharacterName AS PlayCount_CharacterName
					, B.WinCount, B.AccountID AS WinCount_AccountID, B.AccountName AS WinCount_AccountName, B.CharacterID AS WinCount_CharacterID, B.CharacterName AS WinCount_CharacterName
					, C.SemiWinCount, C.AccountID AS SemiWinCount_AccountID, C.AccountName AS SemiWinCount_AccountName, C.CharacterID AS SemiWinCount_CharacterID, C.CharacterName AS SemiWinCount_CharacterName
	FROM @tblPlayCountRank A
		INNER JOIN @tblWinCountRank B ON A.Ranking = B.Ranking	 
		INNER JOIN @tblSemiWinCountRank C ON B.Ranking = C.Ranking

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
