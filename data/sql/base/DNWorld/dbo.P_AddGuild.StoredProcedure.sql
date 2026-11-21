USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuild]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 11
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-09-20
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103234 = ?? ???? ?? ?????.
103235 = ?? ??? ?? ???? ????.
history
6 = ?? ??? ??? ?? Guild ???? ??? ?? ? ?? ??? ?? ??? ?? by ??? at 2011-11-24
9 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30 
10 = ??? ?? ?? ?? by ??? at 2012-07-30
11 = ?? ??? ?? ???? ????? ?? by ??? at 2012-10-10
**/

CREATE PROCEDURE [dbo].[P_AddGuild]
	  @nvcGuildName nvarchar(50) --// ?? ??
	, @nvcDefaultNoticeContent nvarchar(300) --// ??? ?? ??
	, @inbCharacterID bigint --// ?? ??? ??? ID
	, @inyCharacterLevel tinyint --// ?? ??? ??? ??
	, @nvcDefaultSelfIntroduction nvarchar(300) --// ??? ?? ??? ?? ??
	, @intGuildEstablishExpense int --// ?? ?? ??
	, @intMapID int --// ? ID
	, @intGuildID int OUTPUT --// ?? ID
	, @dt2RegisterDate datetime2(0) OUTPUT --// ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @nvcAccountName nvarchar(50);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @dt2RegisterDate = GETDATE();

	/**_# ?? ????? ??? ? ?? ?? ???? ?????.*/
	SELECT @nvcAccountName = AccountName
	FROM dbo.ReservedGuildNames WITH (NOLOCK, INDEX(PK_ReservedGuildNames), FORCESEEK)
	WHERE GuildName = @nvcGuildName
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT > 0 AND NOT EXISTS (
		SELECT *
		FROM dbo.Characters WITH (NOLOCK, INDEX(PK_Characters), FORCESEEK)
		WHERE CharacterID = @inbCharacterID AND AccountName = @nvcAccountName
	)
	BEGIN
		SET @intReturnValue = 103234;
		GOTO ErrorHandler;
	END

	/**_# [Guilds] ???? INSERT???.*/
	/**_## ?? ??? ???? 103234? ?????.*/
	INSERT dbo.Guilds (GuildName, GuildLevel, GuildDepotSize, TotalGuildPoint, RemainGuildPoint, GuildCoin, NoticeContent, RegisterDate, CloseFlag, CloseDate, TotalGuildEXP, DailyStagePoint, StagePointLastModDate, DailyMissionPoint, MissionPointLastModDate)
	VALUES (@nvcGuildName, 1, 0, 0, 0, 0, @nvcDefaultNoticeContent, @dt2RegisterDate, 0, NULL, 0, 0, @dt2RegisterDate, 0, @dt2RegisterDate);

	SET @intGuildID = SCOPE_IDENTITY();

	/**_# [GuildMembers] ???? INSERT???.*/
	/**_## ?? ??? ?? ???? ??? 103235? ?????.*/
	INSERT GuildMembers (GuildID, CharacterID, GuildMemberRoleCode, SelfIntroduction, CharacterJoinLevel)
	VALUES (@intGuildID, @inbCharacterID, 0, @nvcDefaultSelfIntroduction, @inyCharacterLevel);

	/**_# [GuildHistory] ???? INSERT???.*/
	INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, ItemID, ItemLevel, RegisterDate)
	VALUES (@intGuildID, 1, NULL, NULL, NULL, NULL, @dt2RegisterDate)
		, (@intGuildID, 3, @inbCharacterID, @inyCharacterLevel, NULL, NULL, @dt2RegisterDate);

	/**_# ?? ?? ??? ?????.*/
	SET @intGuildEstablishExpense = ABS(@intGuildEstablishExpense);

	EXEC dbo.P_ModCoinForSubProc
		  @inbCharacterID = @inbCharacterID
		, @inyCoinChangeCode = 29
		, @inbCoinChangeFKey = @intGuildID
		, @inbCoin = @intGuildEstablishExpense
		, @intChannelID = 0
		, @intMapID = @intMapID
		, @dt2LogDate = @dt2RegisterDate;

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNF_Guilds_GuildName%'
		SET @intReturnValue = 103234;

	ELSE IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UN_GuildMembers_CharacterID%'
		SET @intReturnValue = 103235;

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
