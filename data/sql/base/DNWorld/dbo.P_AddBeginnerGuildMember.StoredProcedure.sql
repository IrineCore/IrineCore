USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddBeginnerGuildMember]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-06-07
description : ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103235 = ?? ??? ?? ???? ????.
103238 = ??? ???? ????.
103295 = ?? ??? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddBeginnerGuildMember]
	  @inbCharacterID bigint --// ??? ID
    , @inyCharacterLevel tinyint --// ??? ??	
	, @intGuildMaxMemberCount int --// ?? ?? ?? ??
	, @nvcGuildTitle nvarchar(45) = N'BeginnerGuild' --// ????? ????
	, @nvcGuildMemo nvarchar(200) = N'' --// ????? ???	
	, @intJoinedGuildID int = NULL OUTPUT --// ??? ??ID	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @intGuildID int;
DECLARE @dt2RegisterDate datetime2;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @dt2RegisterDate = GETDATE();

	/**_# ???? ??? ??? ??? ?????. */		
	;WITH A AS (
		SELECT GuildID, GuildMemberCount
		FROM dbo.VIX_GuildMemberTotals WITH (NOEXPAND, NOLOCK)
	), B AS (
		SELECT G.GuildID, ISNULL(A.GuildMemberCount,0) AS GuildMemberCount
		FROM dbo.Guilds G WITH (NOLOCK) 
			LEFT OUTER JOIN A ON G.GuildID = A.GuildID
		WHERE G.GuildTypeCode = 1 AND G.CloseFlag = 0
	)
	SELECT TOP 1 @intGuildID = GuildID 
	FROM B 
	WHERE GuildMemberCount < @intGuildMaxMemberCount
	ORDER BY GuildID;	

	IF @@ROWCOUNT = 0
	BEGIN
		/**_# ???? ??? ??? ??? ??? ??? ??? ??? ?????. */		
		DECLARE @intClosedGuildID int;

		SELECT TOP 1 @intClosedGuildID = GuildID
		FROM dbo.Guilds WITH (NOLOCK)
		WHERE GuildTypeCode = 1 AND CloseFlag = 1
		ORDER BY GuildID;		

		IF @@ROWCOUNT = 1
		BEGIN
			/**_# ??? ??? ??? ??? ?? ??? ??????. */		
			UPDATE dbo.Guilds
			SET CloseFlag = 0, CloseDate = NULL, @intGuildID = GuildID
			WHERE GuildID = @intClosedGuildID;				

			BEGIN TRANSACTION;

			/**_# [GuildMembers] ???? INSERT???.*/
			INSERT dbo.GuildMembers (GuildID, CharacterID, GuildMemberRoleCode, SelfIntroduction, RegisterDate)
			SELECT GuildID, @inbCharacterID, 4, N'', @dt2RegisterDate
			FROM dbo.Guilds WITH (UPDLOCK)
			WHERE GuildID = @intClosedGuildID AND CloseFlag = CAST(0 AS bit);

			SET @intJoinedGuildID = @intClosedGuildID;

			/**_## ??? ??? ???? ??? 103238? ?????.*/
			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103238;
				GOTO ErrorHandler;
			END			

			/**_# ?? ??? ?????.*/
			INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, RegisterDate)
			VALUES (@intGuildID, 3, @inbCharacterID, @inyCharacterLevel, @dt2RegisterDate)

			COMMIT TRANSACTION;	
	
		END
		ELSE
		BEGIN
			/**_# ??? ??? ??? ??? ??? ?????? ?????. */		
			DECLARE @intBeginnerGuildCount int;
			DECLARE @nvcNewGuildName nvarchar(50);
			DECLARE @insGuildMark smallint;
			
			SELECT @insGuildMark = CASE CAST ((RAND() * 1000) AS int) % 2 WHEN 0 THEN 268 ELSE 269 END;
		
			SELECT @intBeginnerGuildCount = COUNT(*)
			FROM dbo.Guilds WITH (NOLOCK)
			WHERE GuildTypeCode = 1;		

			SET @intBeginnerGuildCount = @intBeginnerGuildCount + 1;
			SET @nvcNewGuildName = @nvcGuildTitle + CAST(@intBeginnerGuildCount AS nvarchar(5));

			/**_# ?? ??? 0?? ??? ?? 100?? ?? ?????.*/		
			IF @intGuildMaxMemberCount = 0
				SET @intGuildMaxMemberCount = 100;			

			/**_# [Guilds] ???? INSERT???.*/		
			INSERT dbo.Guilds (GuildName, GuildLevel, GuildDepotSize, TotalGuildPoint, RemainGuildPoint, GuildCoin, NoticeContent, RegisterDate, CloseFlag, CloseDate, TotalGuildEXP, DailyStagePoint, StagePointLastModDate, DailyMissionPoint, MissionPointLastModDate, GuildTypeCode, GuildMark, GuildMaxMemberCount)
			VALUES (@nvcNewGuildName, 1, 0, 0, 0, 0, @nvcGuildMemo, @dt2RegisterDate, 0, NULL, 0, 0, @dt2RegisterDate, 0, @dt2RegisterDate, 1, @insGuildMark, @intGuildMaxMemberCount);
			SET @intGuildID = SCOPE_IDENTITY();	

			BEGIN TRANSACTION;

			/**_# [GuildMembers] ???? INSERT???.*/
			INSERT dbo.GuildMembers (GuildID, CharacterID, GuildMemberRoleCode, SelfIntroduction, RegisterDate)
			SELECT GuildID, @inbCharacterID, 4, N'', @dt2RegisterDate
			FROM dbo.Guilds WITH (UPDLOCK)
			WHERE GuildID = @intGuildID AND CloseFlag = CAST(0 AS bit);

			SET @intJoinedGuildID = @intGuildID;

			/**_## ??? ??? ???? ??? 103238? ?????.*/
			IF @@ROWCOUNT = 0
			BEGIN
				SET @intReturnValue = 103238;
				GOTO ErrorHandler;
			END

			/**_# ?? ??? ?????.*/
			INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, RegisterDate)
			VALUES (@intGuildID, 3, @inbCharacterID, @inyCharacterLevel, @dt2RegisterDate)

			COMMIT TRANSACTION;	
		
		END
		
	END
	ELSE
	BEGIN
		BEGIN TRANSACTION;

		/**_# [GuildMembers] ???? INSERT???.*/
		INSERT dbo.GuildMembers (GuildID, CharacterID, GuildMemberRoleCode, SelfIntroduction, RegisterDate)
		SELECT GuildID, @inbCharacterID, 4, N'', @dt2RegisterDate
		FROM dbo.Guilds WITH (UPDLOCK)
		WHERE GuildID = @intGuildID AND CloseFlag = CAST(0 AS bit);

		SET @intJoinedGuildID = @intGuildID;

		/**_## ??? ??? ???? ??? 103238? ?????.*/
		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103238;
			GOTO ErrorHandler;
		END

		/**_# ?? ??? ?????.*/
		INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, RegisterDate)
		VALUES (@intGuildID, 3, @inbCharacterID, @inyCharacterLevel, @dt2RegisterDate)

		COMMIT TRANSACTION;
	END

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_GuildMembers%'
		SET @intReturnValue = 103295;

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
