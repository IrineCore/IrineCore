USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageDarkLairRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : sj-yang
e-mail : therye@eyedentitygames.com
created date : 2010-10-07
description : ???? ?? ???? DOLIS ? ?? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
History :
3 = ?? ?? BlockFlag ?? at 2010-10-21
4 = @bitBlockFlag ?? 1? ?? ??? ???? ?? ?? ?? at 2010-11-03
**/

CREATE PROCEDURE [dbo].[P_GetPageDarkLairRanking]
	  @intMapID int --// ? ID
	, @inyPartyUserCount tinyint --// ?? ?? (1 ~ 4)
	, @inyRankerCount tinyint --// ??? ? ?? ? (1 ~ 255)
	, @bitBlockFlag bit --// ?? ??(0:??, 1:??) 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END;

BEGIN TRY

/**_# CharacterInfo ?? ?? */
	/**_## ? ???? ??? , ? ?? ? ???? | ? ?? */
	/**_### ex) ?????,???????,??,????,????|?????,???????,??,????,????|.... */
	/**_## ?? ???? ?? */	
	/**_### ??, ???, ??????, ?????(?), ???? ??ID, ?????(?????,???????,??,????,????)*/	

	IF @bitBlockFlag = 0
		WITH C1 AS 
		(
			SELECT TOP (@inyRankerCount) RANK() OVER (ORDER BY DLR.PlayRound DESC, PlayTime, DarkLairRankingID) AS Ranking, DarkLairRankingID, DLR.PartyName, DLR.PlayRound, DLR.PlayTime, DLR.BlockFlag
			FROM dbo.DarkLairRanking DLR WITH (NOLOCK)
			WHERE DLR.MapID = @intMapID AND DLR.PartyUserCount = @inyPartyUserCount AND BestFlag = CAST(1 AS bit) AND BlockFlag = 0
		)
		SELECT C1.Ranking, C1.PartyName, C1.PlayRound, C1.PlayTime, C1.DarkLairRankingID
			, STUFF((SELECT 
					  N'|' + CA.CharacterName
					, N',' + CAST(CA.CharacterID AS nvarchar(25))
					, N',' + CAST(CharacterLevel AS nvarchar(20))
					, N',' + CAST(JobCode AS nvarchar(20))
					, N',' + CAST(C1.BlockFlag AS nvarchar(2))				
				FROM dbo.DarkLairPartyMembers DLM WITH (NOLOCK) 
					INNER JOIN dbo.Characters CA WITH (NOLOCK) ON DLM.CharacterID = CA.CharacterID 
				WHERE DLM.DarkLairRankingID = C1.DarkLairRankingID 
				  AND DLM.CompleteFlag = CAST(1 AS bit)
				ORDER BY DLM.CharacterID
				FOR XML PATH(N'')), 1, 1, N'') AS CharacterInfo
		FROM C1;

	ELSE
		WITH C1 AS 
		(
			SELECT TOP (@inyRankerCount) RANK() OVER (ORDER BY DLR.PlayRound DESC, PlayTime, DarkLairRankingID) AS Ranking, DarkLairRankingID, DLR.PartyName, DLR.PlayRound, DLR.PlayTime, DLR.BlockFlag
			FROM dbo.DarkLairRanking DLR WITH (NOLOCK)
			WHERE DLR.MapID = @intMapID AND DLR.PartyUserCount = @inyPartyUserCount AND BlockFlag = 1
		)
		SELECT C1.Ranking, C1.PartyName, C1.PlayRound, C1.PlayTime, C1.DarkLairRankingID
			, STUFF((SELECT 
					  N'|' + CA.CharacterName
					, N',' + CAST(CA.CharacterID AS nvarchar(25))
					, N',' + CAST(CharacterLevel AS nvarchar(20))
					, N',' + CAST(JobCode AS nvarchar(20))
					, N',' + CAST(C1.BlockFlag AS nvarchar(2))				
				FROM dbo.DarkLairPartyMembers DLM WITH (NOLOCK) 
					INNER JOIN dbo.Characters CA WITH (NOLOCK) ON DLM.CharacterID = CA.CharacterID
				WHERE DLM.DarkLairRankingID = C1.DarkLairRankingID 
				  AND DLM.CompleteFlag = CAST(1 AS bit)
				ORDER BY DLM.CharacterID
				FOR XML PATH(N'')), 1, 1, N'') AS CharacterInfo
		FROM C1;

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
