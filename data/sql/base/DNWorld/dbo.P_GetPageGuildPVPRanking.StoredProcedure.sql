USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageGuildPVPRanking]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-26
description : ????? PVP ?? ??? ????? ?????? ?????.
result set : CharacterName/nvarchar, JobCode/tinyint, GuildName/nvarchar, PVPExp/int, [Kill]/int, Death/int, PVPClassRank/int, PVPClassRatio/float, TotalRank/bigint, CharacterClassRank/bigint, CharacterSubClassRank/bigint, ChangedTotalRank/bigint, ChangedCharacterClassRank/bigint, ChangedCharacterSubClassRank/bigint, CharacterLevel/int, PVPLevel/int		
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103342 = PVP ????? ????.
**/

CREATE PROCEDURE [dbo].[P_GetPageGuildPVPRanking]
	  @intPageNumber int --// ??? ??.
	, @intPageSize int --// ??? ???.	
	, @intGuildID int = NULL --// ????ID
	, @nvcGuildName nvarchar(50) = NULL --// ?????
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
	IF @intPageNumber = 1
		EXEC @intReturnValue = dbo.P_GetCountPVPRanking			  
			 @intGuildID = @intGuildID 
		   , @nvcGuildName = @nvcGuildName 
		   , @inbTotalCount = @inbTotalCount OUTPUT;

	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @nvcSelectStmt = N'CharacterID, CharacterName, CharacterClassCode, CharacterSubClassCode, JobCode, GuildID, GuildName, PVPExp, [Kill], Death, PVPClassRank, PVPClassRatio, TotalRank, CharacterClassRank, CharacterSubClassRank,
			CASE WHEN ChangedTotalRank IS NULL THEN 2 
				 WHEN ChangedTotalRank > 0 THEN 1 
				 WHEN ChangedTotalRank < 0 THEN -1 
				 ELSE 0 END AS ChangedTotalRank,
			CASE WHEN ChangedCharacterClassRank IS NULL THEN 2 
				 WHEN ChangedCharacterClassRank > 0 THEN 1 
				 WHEN ChangedCharacterClassRank < 0 THEN -1 
				 ELSE 0 END AS ChangedCharacterClassRank,
 			CASE WHEN ChangedCharacterSubClassRank IS NULL THEN 2 
				 WHEN ChangedCharacterSubClassRank > 0 THEN 1 
				 WHEN ChangedCharacterSubClassRank < 0 THEN -1 
				 ELSE 0 END AS ChangedCharacterSubClassRank';

	SET @nvcSelectStmt2 = N'PC.CharacterID, PC.CharacterName, PC.JobCode, PC.GuildName, PC.PVPExp, PC.[Kill], PC.Death, PC.PVPClassRank, PC.PVPClassRatio, PC.TotalRank, PC.CharacterClassRank, PC.CharacterSubClassRank, PC.ChangedTotalRank, PC.ChangedCharacterClassRank, PC.ChangedCharacterSubClassRank, CS.CharacterLevel, ISNULL(PS.PVPLevel, 0) AS PVPLevel'

	SET @intColumnCount = 17;

	SET @inbNullBitmap = 		
		CASE WHEN @intGuildID IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @nvcGuildName IS NOT NULL THEN POWER(2, 1) ELSE 0 END;

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @intPageNumber = ISNULL(@intPageNumber, 1);
	SET @intPageSize = ISNULL(@intPageSize, 1000);
	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY TotalRank DESC) AS rowNumber, ' + @nvcSelectStmt + N'
	FROM dbo.PVPRanking WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'GuildID', N'@intGuildID', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'GuildName', N'@nvcGuildName', @inbNullBitmap, 2, @intColumnCount, '=') + N'
)
SELECT ' + @nvcSelectStmt2 + N'
FROM PagingCTE PC INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON PC.CharacterID = CS.CharacterID
	 LEFT OUTER JOIN dbo.PVPScores PS ON PC.CharacterID = PS.CharacterID
WHERE rowNumber BETWEEN (((@intPageNumber - 1) * @intPageSize) + 1) AND (@intPageNumber * @intPageSize)
ORDER BY rowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @intGuildID int
			, @nvcGuildName nvarchar(50)			
			, @intPageNumber int
			, @intPageSize int
		'		
		, @intGuildID = @intGuildID
		, @nvcGuildName = @nvcGuildName		
		, @intPageNumber = @intPageNumber
		, @intPageSize = @intPageSize;

	/**_## ??? ???? ?? ???? ???, ?(?) ?????.*/
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
