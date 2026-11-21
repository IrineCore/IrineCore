USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPagePVPRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 32
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-26
description : PVP ?? ??? ????? ?????? ?????.
result set : CharacterName/nvarchar, JobCode/tinyint, GuildName/nvarchar, PVPExp/int, Kill/int, Death/int, PVPClassRank/int, PVPClassRatio/float,  Rank/bigint, ChangedRank/int, CharacterLevel/int, PVPLevel/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103342 = PVP ????? ????.
**/

CREATE PROCEDURE [dbo].[P_GetPagePVPRanking]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.	
	, @inyCharacterClassCode tinyint = NULL --// ??? ?????
	, @inyCharacterSubClassCode tinyint = NULL --// 2? ?? ??	
	, @nvcGuildName nvarchar(50) = NULL --// ???	
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
			FROM dbo.PVPRankingCount WITH(NOLOCK)
			WHERE '

		IF @inyCharacterClassCode IS NOT NULL
			SET @nvcSelectStmt += N'GuildName IS NULL AND CharacterClassCode = @inyCharacterClassCode'
		ELSE IF @inyCharacterSubClassCode IS NOT NULL
			SET @nvcSelectStmt += N'GuildName IS NULL AND CharacterSubClassCode = @inyCharacterSubClassCode'
		ELSE IF @nvcGuildName IS NOT NULL
			SET @nvcSelectStmt += N'GuildName = @nvcGuildName'
		ELSE 
			SET @nvcSelectStmt += N'GuildName IS NULL AND Total = 1'		
		
		
		EXEC dbo.sp_executesql @nvcSelectStmt, N'
			  @inyCharacterClassCode tinyint
			, @inyCharacterSubClassCode tinyint
			, @nvcGuildName nvarchar(50)
			, @inbTotalCount bigint OUTPUT'		
		, @inyCharacterClassCode = @inyCharacterClassCode
		, @inyCharacterSubClassCode = @inyCharacterSubClassCode		
		, @nvcGuildName = @nvcGuildName
		, @inbTotalCount = @inbTotalCount OUTPUT;
	--END

	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @nvcSelectStmt = N'	
	SELECT CharacterName, JobCode, GuildName, PVPExp, [Kill], Death
	     , CharacterClassRank AS PVPClassRank
	     , PVPClassRatio'
	IF @inyCharacterClassCode IS NOT NULL
	     SET @nvcSelectStmt += N'
	     , CharacterClassRank AS Rank
	     , CASE WHEN ChangedCharacterClassRank IS NULL THEN 2 
				WHEN ChangedCharacterClassRank > 0 THEN 1 
				WHEN ChangedCharacterClassRank < 0 THEN -1 
				ELSE 0 END AS ChangedRank'
	ELSE IF @inyCharacterSubClassCode IS NOT NULL
		 SET @nvcSelectStmt += N'
		 , CharacterSubClassRank AS Rank
		 , CASE WHEN ChangedCharacterSubClassRank IS NULL THEN 2 
				WHEN ChangedCharacterSubClassRank > 0 THEN 1 
				WHEN ChangedCharacterSubClassRank < 0 THEN -1 
				ELSE 0 END AS ChangedRank'
 	ELSE
		 SET @nvcSelectStmt += N'
		 , TotalRank AS Rank
		 , CASE WHEN ChangedTotalRank IS NULL THEN 2 
				WHEN ChangedTotalRank > 0 THEN 1 
				WHEN ChangedTotalRank < 0 THEN -1 
				ELSE 0 END AS ChangedRank'
	SET @nvcSelectStmt += N'
	     , CharacterLevel
	     , PVPLevel'
	IF @inyCharacterClassCode IS NOT NULL
		SET @nvcSelectStmt += N' FROM dbo.PVPRanking WITH (NOLOCK, INDEX(IX_NN_PVPRanking_CharacterClassRank), FORCESEEK)'
	ELSE IF @inyCharacterSubClassCode IS NOT NULL
		SET @nvcSelectStmt += N' FROM dbo.PVPRanking WITH (NOLOCK, INDEX(IX_NN_PVPRanking_CharacterSubClassRank), FORCESEEK)'
	ELSE IF @nvcGuildName IS NOT NULL
		SET @nvcSelectStmt += N' FROM dbo.PVPRanking WITH (NOLOCK, INDEX(IX_NN_PVPRanking_GuildName), FORCESEEK)'
	ELSE 
		SET @nvcSelectStmt += N' FROM dbo.PVPRanking WITH (NOLOCK, INDEX(IX_NC_PVPRanking_TotalRank), FORCESEEK)'
	SET @nvcSelectStmt += N' WHERE '
	IF @inyCharacterClassCode IS NOT NULL
		SET @nvcSelectStmt += N'CharacterClassRank BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize) AND CharacterClassCode = @inyCharacterClassCode';
	ELSE IF @inyCharacterSubClassCode IS NOT NULL
		SET @nvcSelectStmt += N'CharacterSubClassRank BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize) AND CharacterSubClassCode = @inyCharacterSubClassCode';
	ELSE IF @nvcGuildName IS NOT NULL
		SET @nvcSelectStmt += N'GuildName = @nvcGuildName';
	ELSE 
		SET @nvcSelectStmt += N'TotalRank BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)';

	

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcSelectStmt
		, N'
			  @inyCharacterClassCode tinyint
			, @inyCharacterSubClassCode tinyint
			, @nvcGuildName nvarchar(50)
			, @intPageNumber int
			, @intPageSize int
		'		
		, @inyCharacterClassCode = @inyCharacterClassCode
		, @inyCharacterSubClassCode = @inyCharacterSubClassCode		
		, @nvcGuildName = @nvcGuildName
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ???, 103342 ? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103342;
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
