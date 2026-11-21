USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPagePVPLadderRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 41
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-26
description : PVP ?? ?? ??? ????? ?????? ?????.
result set : CharacterName/nvarchar, JobCode/tinyint, GuildName/nvarchar, PVPLadderGradePoint/int, Win/int, Lose/int, Draw/int, PVPExp/int, PVPClassRank/int, PVPClassRatio/float, Rank/bigint, ChangedRank/int, CharacterLevel/int, PVPLevel/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103343 = PVP ?? ????? ????.
**/

CREATE PROCEDURE [dbo].[P_GetPagePVPLadderRanking]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.	
	, @inyPVPLadderCode tinyint = 1 --// PVP ?? ?? ?? (1=1:1, 2=2:2, 3=3:3)
	, @inyCharacterClassCode tinyint = NULL --// ??? ?????
	, @inyCharacterSubClassCode tinyint = NULL --// 2? ?? ??	
	, @nvcGuildName nvarchar(50) = N'' --// ???
	, @inbTotalCount bigint OUTPUT --// ?? ?? ?? ?? ?????. (1?????? ?????.)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@nvcSelectStmt nvarchar(max),
	@nvcSelectStmt2 nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	--IF @intPageNumber = 1
	--BEGIN
		SET @nvcSelectStmt = N'
			SELECT @inbTotalCount = TotalCount
			FROM dbo.PVPLadderRankingCount WITH(NOLOCK)
			WHERE '

		IF @inyCharacterClassCode IS NOT NULL
			SET @nvcSelectStmt += N'GuildName IS NULL AND CharacterClassCode = @inyCharacterClassCode'
		ELSE IF @inyCharacterSubClassCode IS NOT NULL
			SET @nvcSelectStmt += N'GuildName IS NULL AND CharacterSubClassCode = @inyCharacterSubClassCode'
		ELSE IF @nvcGuildName IS NOT NULL
			SET @nvcSelectStmt += N'GuildName = @nvcGuildName'
		ELSE 
			SET @nvcSelectStmt += N'GuildName IS NULL AND Total = 1'
			
		IF @inyPVPLadderCode IS NOT NULL
			SET @nvcSelectStmt += N' AND PVPLadderCode = @inyPVPLadderCode '
		
		EXEC dbo.sp_executesql @nvcSelectStmt, N'
			  @inyCharacterClassCode tinyint
			, @inyCharacterSubClassCode tinyint
			, @nvcGuildName nvarchar(50)
			, @inyPVPLadderCode tinyint
			, @inbTotalCount bigint OUTPUT'		
		, @inyCharacterClassCode = @inyCharacterClassCode
		, @inyCharacterSubClassCode = @inyCharacterSubClassCode		
		, @nvcGuildName = @nvcGuildName
		, @inyPVPLadderCode = @inyPVPLadderCode
		, @inbTotalCount = @inbTotalCount OUTPUT;
 
	--END
	
	SET @inyPVPLadderCode = ISNULL(@inyPVPLadderCode, 1);
	
	SET @nvcSelectStmt = N'	
	SELECT PL.CharacterName, PL.JobCode, PL.GuildName, PL.PVPLadderGradePoint, PL.Win, PL.Lose, PL.Draw, PS.PVPExp
	     , PL.CharacterClassRank AS PVPClassRank
	     , PL.PVPClassRatio'
	     
	IF @inyCharacterClassCode IS NOT NULL
	     SET @nvcSelectStmt += N'
	     , PL.CharacterClassRank AS Rank
	     , CASE WHEN PL.ChangedCharacterClassRank IS NULL THEN 2 
				WHEN PL.ChangedCharacterClassRank > 0 THEN 1 
				WHEN PL.ChangedCharacterClassRank < 0 THEN -1 
				ELSE 0 END AS ChangedRank'
	ELSE IF @inyCharacterSubClassCode IS NOT NULL
		 SET @nvcSelectStmt += N'
		 , PL.CharacterSubClassRank AS Rank
		 , CASE WHEN PL.ChangedCharacterSubClassRank IS NULL THEN 2 
				WHEN PL.ChangedCharacterSubClassRank > 0 THEN 1 
				WHEN PL.ChangedCharacterSubClassRank < 0 THEN -1 
				ELSE 0 END AS ChangedRank'
 	ELSE
		 SET @nvcSelectStmt += N'
		 , PL.TotalRank AS Rank
		 , CASE WHEN PL.ChangedTotalRank IS NULL THEN 2 
				WHEN PL.ChangedTotalRank > 0 THEN 1 
				WHEN PL.ChangedTotalRank < 0 THEN -1 
				ELSE 0 END AS ChangedRank'
				
	SET @nvcSelectStmt += N'
	     , CS.CharacterLevel
	     , PS.PVPLevel'
	
	IF @inyCharacterClassCode IS NOT NULL
		SET @nvcSelectStmt += N' FROM dbo.PVPLadderRanking PL WITH (NOLOCK, INDEX(IX_NN_PVPLadderRanking_CharacterClassRank), FORCESEEK)
		 INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON PL.CharacterID = CS.CharacterID
		 LEFT OUTER JOIN dbo.PVPScores PS WITH (NOLOCK) ON PL.CharacterID = PS.CharacterID'
	ELSE IF @inyCharacterSubClassCode IS NOT NULL
		SET @nvcSelectStmt += N' FROM dbo.PVPLadderRanking PL WITH (NOLOCK, INDEX(IX_NN_PVPLadderRanking_CharacterSubClassRank), FORCESEEK) 
		 INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON PL.CharacterID = CS.CharacterID
		 LEFT OUTER JOIN dbo.PVPScores PS WITH (NOLOCK) ON PL.CharacterID = PS.CharacterID'
	ELSE IF @nvcGuildName IS NOT NULL
		SET @nvcSelectStmt += N' FROM dbo.PVPLadderRanking PL WITH (NOLOCK, INDEX(IX_NN_PVPLadderRanking_GuildName), FORCESEEK) 
		 INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON PL.CharacterID = CS.CharacterID
		 LEFT OUTER JOIN dbo.PVPScores PS WITH (NOLOCK) ON PL.CharacterID = PS.CharacterID'
	ELSE 
		SET @nvcSelectStmt += N' FROM dbo.PVPLadderRanking PL WITH (NOLOCK, INDEX(IX_NC_PVPLadderRanking_TotalRank), FORCESEEK) 
		 INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON PL.CharacterID = CS.CharacterID
		 LEFT OUTER JOIN dbo.PVPScores PS WITH (NOLOCK) ON PL.CharacterID = PS.CharacterID'
		
	SET @nvcSelectStmt += N' WHERE '
	IF @inyCharacterClassCode IS NOT NULL
		SET @nvcSelectStmt += N'PL.CharacterClassRank BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize) AND PL.CharacterClassCode = @inyCharacterClassCode';
	ELSE IF @inyCharacterSubClassCode IS NOT NULL
		SET @nvcSelectStmt += N'PL.CharacterSubClassRank BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize) AND PL.CharacterSubClassCode = @inyCharacterSubClassCode';
	ELSE IF @nvcGuildName IS NOT NULL
		SET @nvcSelectStmt += N'PL.GuildName = @nvcGuildName';
	ELSE 
		SET @nvcSelectStmt += N'PL.TotalRank BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)';	
	
	IF @inyPVPLadderCode IS NOT NULL
			SET @nvcSelectStmt += N' AND PL.PVPLadderCode = @inyPVPLadderCode '
		
	IF @inyCharacterClassCode IS NOT NULL
		SET @nvcSelectStmt += N' ORDER BY PL.CharacterClassRank '
	ELSE IF @inyCharacterSubClassCode IS NOT NULL
		SET @nvcSelectStmt += N' ORDER BY PL.CharacterSubClassRank '
	ELSE
		SET @nvcSelectStmt += N' ORDER BY PL.TotalRank '

	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);
	
	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcSelectStmt
		, N'
			  @inyCharacterClassCode tinyint
			, @inyCharacterSubClassCode tinyint
			, @nvcGuildName nvarchar(50)
			, @inyPVPLadderCode tinyint
			, @intPageNumber int
			, @intPageSize int
		'		
		, @inyCharacterClassCode = @inyCharacterClassCode
		, @inyCharacterSubClassCode = @inyCharacterSubClassCode		
		, @nvcGuildName = @nvcGuildName
		, @inyPVPLadderCode = @inyPVPLadderCode
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ???, 103343? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN		
		SET @intReturnValue = 103343;
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
