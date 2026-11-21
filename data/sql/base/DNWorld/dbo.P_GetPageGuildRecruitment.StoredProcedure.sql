USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageGuildRecruitment]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 32
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-18

description :
	??? ?? ?? ??? ?? ??? ?????? ?????.
result set :
	@intVersion < 20 : GuildID/int, GuildName/nvarchar(50), GuildLevel/tinyint, GuildMark/smallint, GuildMarkBG/smallint, GuildMarkBorder/smallint, GuildIntroduction/nvarchar(max), GuildMemberCount/int, GuildMaxMemberCount/int
	@intVersion = 20 : GuildID/int, GuildName/nvarchar(50), GuildLevel/tinyint, GuildMark/smallint, GuildMarkBG/smallint, GuildMarkBorder/smallint, GuildIntroduction/nvarchar(max), GuildMemberCount/int, GuildMaxMemberCount/int, PurposeCode/tinyint, Homepage/nvarchar(max)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	20 = input parameter ?? (SP ??, ????, ???, ????), result set ?? (????, ????)     by ??? at 2012-05-08
	21 = GuildRecruitments ???? MinLevel, MaxLevel ?? ??  by ??? at 2012-05-10
	22 = ?? ???? ?? ??? ?? ??     by ??? at 2012-05-24
	23 = input parameter ?? (SP ??)     by ??? at 2012-05-24
	24 = 23 ?? ??     by ??? at 2012-05-24
	28 = ?? ?? (CloseFlag ??? ??? ??)     by ??? at 2013-05-09
	29 = ?? ?? (?? Loop Join, Force Order ??)     by ??? at 2013-05-15
	30 = 29? ??     by ??? at 2013-05-15
	31 = ??    by ??? at 2013-06-25
**/

CREATE PROCEDURE [dbo].[P_GetPageGuildRecruitment]
	  @intStartNumber int --// ?? ??.
	, @intEndNumber int --// ? ??.
	, @inyCharacterClassCode tinyint --// 1=???, 2=??, 3=????, 4=???, 5=????
	, @inyCharacterLevel tinyint -- ??? ??
	, @intVersion int = 19 --// SP ??
	, @inyPurposeCode tinyint = 1 --// ???? (0=??, 1=??, 2=??, 3=???, 4=????)
	, @nvcGuildName nvarchar(50) = NULL --// ???
	, @intSortTypeCode int = 1 --// ???? (1=???? ????, 2=???? ????, 3=?????? ????, 4=?????? ????, 5=??? ????, 6=??? ????)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @nvcStmt nvarchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF @intVersion < 20
	BEGIN
		;WITH PagingCTE AS (
			SELECT ROW_NUMBER() OVER(ORDER BY GuildID) AS PageNo, GuildID, GuildIntroduction 
			FROM dbo.GuildRecruitments WITH (NOLOCK, INDEX(PK_GuildRecruitments))
			WHERE CharacterClassCodeBitmap & POWER(2, @inyCharacterClassCode - 1) > 0
			  and @inyCharacterLevel between MinLevel AND MaxLevel
		)
		SELECT A.GuildID, B.GuildName, B.GuildLevel, B.GuildMark, B.GuildMarkBG, B.GuildMarkBorder, A.GuildIntroduction, CAST(C.GuildMemberCount AS int) AS GuildMemberCount, B.GuildMaxMemberCount
		FROM PagingCTE A 
			INNER LOOP JOIN dbo.Guilds B WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON A.GuildID = B.GuildID
			INNER LOOP JOIN dbo.VIX_GuildMemberTotals C WITH (NOEXPAND, NOLOCK) ON B.GuildID = C.GuildID 
		WHERE A.PageNo BETWEEN @intStartNumber AND @intEndNumber
		OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
	END
	ELSE IF @intVersion = 20
	BEGIN
		SET @nvcStmt = N'SET NOCOUNT ON;

			WITH T AS ('
		IF @intSortTypeCode = 1      -- ???? ????
			SET @nvcStmt = @nvcStmt + N'
				SELECT ROW_NUMBER() OVER(ORDER BY A.GuildLevel DESC) AS PageNo'
		ELSE IF @intSortTypeCode = 2 -- ???? ????
			SET @nvcStmt = @nvcStmt + N'
				SELECT ROW_NUMBER() OVER(ORDER BY A.GuildLevel) AS PageNo'
		ELSE IF @intSortTypeCode = 3 -- ?????? ????
			SET @nvcStmt = @nvcStmt + N'
				SELECT ROW_NUMBER() OVER(ORDER BY V.GuildMemberCount DESC) AS PageNo'
		ELSE IF @intSortTypeCode = 4 -- ?????? ????
			SET @nvcStmt = @nvcStmt + N'
				SELECT ROW_NUMBER() OVER(ORDER BY V.GuildMemberCount) AS PageNo'
		ELSE IF @intSortTypeCode = 5 -- ??? ????
			SET @nvcStmt = @nvcStmt + N'
				SELECT ROW_NUMBER() OVER(ORDER BY A.GuildName DESC) AS PageNo'
		ELSE IF @intSortTypeCode = 6 -- ??? ????
			SET @nvcStmt = @nvcStmt + N'
				SELECT ROW_NUMBER() OVER(ORDER BY A.GuildName) AS PageNo'
		SET @nvcStmt = @nvcStmt + N'
				     , A.GuildID, A.GuildIntroduction, A.PurposeCode, A.GuildHomepageURLFlag'
		IF @intSortTypeCode IN (3, 4)
			SET @nvcStmt = @nvcStmt + N'
					 , CAST(V.GuildMemberCount AS int) AS GuildMemberCount'
		SET @nvcStmt = @nvcStmt + N'
				  FROM dbo.GuildRecruitments A WITH (NOLOCK, INDEX(PK_GuildRecruitments))'
		IF @intSortTypeCode IN (3, 4)
			SET @nvcStmt = @nvcStmt + N'
					   INNER JOIN dbo.VIX_GuildMemberTotals V WITH (NOEXPAND, NOLOCK) ON (A.GuildID = V.GuildID )'
		SET @nvcStmt = @nvcStmt + N'
				 WHERE (CharacterClassCodeBitmap & POWER(2, @inyCharacterClassCode - 1) > 0)
				   AND (@inyCharacterLevel BETWEEN A.MinLevel AND A.MaxLevel)
				   AND (@inyPurposeCode = 0 OR A.PurposeCode = @inyPurposeCode)'

		IF @nvcGuildName IS NOT NULL
			SET @nvcStmt = @nvcStmt + N'
				   AND (A.GuildName LIKE N''%'' + @nvcGuildName + N''%'')'

		SET @nvcStmt = @nvcStmt + N'
			)
			SELECT C.GuildID, C.GuildName, C.GuildLevel, C.GuildMark, C.GuildMarkBG, C.GuildMarkBorder, T.GuildIntroduction'
		IF @intSortTypeCode IN (3, 4)
			SET @nvcStmt = @nvcStmt + N'
				 , T.GuildMemberCount'
		ELSE IF @intSortTypeCode IN (1, 2, 5, 6)
			SET @nvcStmt = @nvcStmt + N'
				 , V.GuildMemberCount'
		SET @nvcStmt = @nvcStmt + N'
				 , C.GuildMaxMemberCount, T.PurposeCode, C.GuildHomepageURL, E.CharacterName
			  FROM T
					INNER JOIN dbo.Guilds C WITH (NOLOCK) ON (T.GuildID = C.GuildID)
				    INNER JOIN dbo.GuildMembers D WITH (NOLOCK) ON (C.GuildID = D.GuildID AND D.GuildMemberRoleCode = 0)
				    INNER JOIN dbo.Characters E WITH (NOLOCK) ON (D.CharacterID = E.CharacterID)'
		IF @intSortTypeCode IN (1, 2, 5, 6)
			SET @nvcStmt = @nvcStmt + N'
				    INNER JOIN dbo.VIX_GuildMemberTotals V WITH (NOEXPAND, NOLOCK) ON (T.GuildID = V.GuildID )'
		SET @nvcStmt = @nvcStmt + N'
			 WHERE PageNo BETWEEN @intStartNumber AND @intEndNumber
			OPTION (MAXDOP 1);'

		EXEC dbo.sp_executesql @nvcStmt, N'
			  @intStartNumber int
			, @intEndNumber int
			, @inyCharacterClassCode tinyint
			, @inyCharacterLevel tinyint
			, @inyPurposeCode tinyint
			, @nvcGuildName nvarchar(50)
			, @intSortTypeCode int'
			, @intStartNumber = @intStartNumber
			, @intEndNumber = @intEndNumber
			, @inyCharacterClassCode = @inyCharacterClassCode
			, @inyCharacterLevel = @inyCharacterLevel
			, @inyPurposeCode = @inyPurposeCode
			, @nvcGuildName = @nvcGuildName
			, @intSortTypeCode = @intSortTypeCode;
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
