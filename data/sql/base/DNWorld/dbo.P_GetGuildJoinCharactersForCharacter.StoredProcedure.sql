USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuildJoinCharactersForCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 16
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-21

description :
	??? ???? ?? ??? ?? ??? ?????.
result set :
	@intVersion < 14 : GuildID/int, GuildName/nvarchar(50), GuildLevel/tinyint, GuildMark/smallint, GuildMarkBG/smallint, GuildMarkBorder/smallint, GuildIntroduction/nvarchar, GuildMemberCount/int, GuildMaxMemberCount/int
	@intVersion = 14 : GuildID/int, GuildName/nvarchar(50), GuildLevel/tinyint, GuildMark/smallint, GuildMarkBG/smallint, GuildMarkBorder/smallint, GuildIntroduction/nvarchar, GuildMemberCount/int, GuildMaxMemberCount/int, PurposeCode/tinyint , GuildHomepageURL/nvarchar(max), CharacterName/nvarchar(30)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	13 = ???? ? ???? ??     by ??? at 2012-05-24
	14 = SP ?? ??     by ??? at 2012-05-24
	15 = SP ?? ??? ??     by ??? at 2012-05-24
	16 = result set ?? (???)     by ??? at 2012-05-24
**/

CREATE PROCEDURE [dbo].[P_GetGuildJoinCharactersForCharacter]
	  @inbCharacterID bigint --//  ?? ??? CharacterID
	, @intVersion int = 13 --// SP ??
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

	/**_# ?? ?? ??? ?? ??? ?????.*/
	IF @intVersion < 14
	BEGIN
		SELECT B.GuildID, B.GuildName, B.GuildLevel, B.GuildMark, B.GuildMarkBG, B.GuildMarkBorder, C.GuildIntroduction, D.GuildMemberCount, B.GuildMaxMemberCount
		FROM dbo.GuildJoinCharacters A WITH (NOLOCK, INDEX(IX_NN_GuildJoinCharacters_CharacterID), FORCESEEK)
			INNER LOOP JOIN dbo.Guilds B WITH (NOLOCK, INDEX(PK_Guilds), FORCESEEK) ON A.GuildID = B.GuildID
			INNER LOOP JOIN dbo.GuildRecruitments C WITH (NOLOCK, INDEX(PK_GuildRecruitments), FORCESEEK) ON B.GuildID = C.GuildID
			INNER LOOP JOIN dbo.VIX_GuildMemberTotals D WITH (NOEXPAND, NOLOCK) ON C.GuildID = D.GuildID 
		WHERE A.CharacterID = @inbCharacterID	
		OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
	END
	ELSE IF @intVersion = 14
	BEGIN
		SELECT B.GuildID, B.GuildName, B.GuildLevel, B.GuildMark, B.GuildMarkBG, B.GuildMarkBorder, C.GuildIntroduction, D.GuildMemberCount, B.GuildMaxMemberCount, C.PurposeCode, CASE WHEN C.GuildHomepageURLFlag = CONVERT(bit, 0) THEN N'' ELSE B.GuildHomepageURL END AS GuildHomepageURL, F.CharacterName
		FROM dbo.GuildJoinCharacters A WITH (NOLOCK)
			INNER JOIN dbo.Guilds B WITH (NOLOCK) ON (A.GuildID = B.GuildID)
			INNER JOIN dbo.GuildRecruitments C WITH (NOLOCK) ON (B.GuildID = C.GuildID)
			INNER JOIN dbo.VIX_GuildMemberTotals D WITH (NOEXPAND, NOLOCK) ON (C.GuildID = D.GuildID )
			INNER JOIN dbo.GuildMembers E WITH (NOLOCK) ON (B.GuildID = E.GuildID AND E.GuildMemberRoleCode = 0)
			INNER JOIN dbo.Characters F WITH (NOLOCK) ON (E.CharacterID = F.CharacterID)
		WHERE A.CharacterID = @inbCharacterID	
		OPTION (FORCE ORDER, LOOP JOIN);
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
