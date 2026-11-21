USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarFinalResults]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : wgkim@eyedentitygames
created date : 2011-03-08

description :
    ??? ?? ?? ??
		Case 1) @insGuildWarScheduleID = 0, @insVersion = 0 - ?? ?? 16? ?? ?? ??? ??
		Case 2) @insGuildWarScheduleID = 0, @insVersion = 1 - ?? ?? ?? ?? ??
		Case 3) @insGuildWarScheduleID > 0, @insVersion = 0 - ??? ?? ?? ?? ?? (??? ??)
		Case 4) @insGuildWarScheduleID > 0, @insVersion = 1 - ??? ?? ?? ?? ?? (?? ?? - ???? ?? ??)
result set :
	Case 1) @insGuildWarScheduleID = 0, @insVersion = 0
		GuildID / int
		RemainGuildPoint / int
	Case 2) @insGuildWarScheduleID = 0, @insVersion = 1
		GuildWarScheduleID / smallint
		GuildID / int
	Case 3) @insGuildWarScheduleID > 0, @insVersion = 0
		GuildID / int
		GuildName / nvarchar(50)
		MatchSequence / tinyint
		MatchTypeCode / tinyint
		MatchResultCode / tinyint
	Case 4) @insGuildWarScheduleID > 0, @insVersion = 1
		GuildID / int
		GuildName / nvarchar(50)
		MatchSequence / tinyint
		MatchTypeCode / tinyint
		MatchResultCode / tinyint
		GuildMark / smallint
		GuildMarkBG / smallint
		GuildMarkBorder / smallint
return value :
    0 = ??? ????.
    1 = ????? Commit? ? ?? ?????. ????? Rollback???.
    100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
    4 = ?? ??
	5 = result set ?? (@insVersion = 1)
	6 = @insGuildWarScheduleID ? 0 ? ???? ?? ??? ?? ?? ?? ??     by 20110919.wgkim
	7 = ??? ???? ?? ?? ?? ?? ??     by 20110920.wgkim
	8~9 = ?? ?? 16? ?? ?? ??? ?? ??     by 20110923.wgkim
	10 = ????? ??? ?? ???? ????? ?? by ??? at 2011-12-06
	11 = ?? ???? ??     by ??? at 2012-06-11
	12 = Join ???? GuildWarScheduleID ? ???? ??? ???? ???? ?? ??     by ??? at 2012-06-20
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarFinalResults]
	  @insGuildWarScheduleID smallint = 0 --// ??? ??
	, @insVersion smallint = 0 --// Version
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

	IF @insGuildWarScheduleID = 0
	BEGIN

		SELECT @insGuildWarScheduleID = MAX(GuildWarScheduleID)
		  FROM dbo.GuildWarFinalResults WITH (NOLOCK)
		 WHERE MatchTypeCode = 1
		   AND MatchResultCode = 1;

		IF @insVersion = 1
		BEGIN

			/**_# ??? ???? ?? ?? ??? ?????. */
			SELECT TOP 1 GuildWarScheduleID, GuildID
			  FROM dbo.GuildWarFinalResults WITH (NOLOCK)
			 WHERE GuildWarScheduleID = @insGuildWarScheduleID
			   AND MatchTypeCode = 1
			   AND MatchResultCode = 1;
		END
		ELSE
		BEGIN

			/**_# ??? ???? ?? ??? ??? ?????. */
			SELECT GWFR.GuildID, CASE WHEN TotalGuildEXP <= 0 THEN G.RemainGuildPoint ELSE G.TotalGuildEXP END AS RemainGuildPoint
			  FROM dbo.GuildWarFinalResults GWFR WITH (NOLOCK)
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (GWFR.GuildID = G.GuildID)
			 WHERE GWFR.GuildWarScheduleID = @insGuildWarScheduleID;

		END

	END
	ELSE
	BEGIN

		/**_# ??? ?? ??? ?????. */
		IF @insVersion = 1
		BEGIN
			SELECT G.GuildID                -- ?? ID
				 , G.GuildName              -- ???
				 , GWFR.MatchSequence       -- ?? ??
				 , GWFR.MatchTypeCode       -- 0=???, 1=??, 2=4?, 3=8?, 4=16?
				 , GWFR.MatchResultCode     -- 0=???, 1=??, 2=??, 3=??
				 , G.GuildMark				-- ?? ??
				 , G.GuildMarkBG			-- ?? ?? ??
				 , G.GuildMarkBorder		-- ?? ?? ???
			  FROM dbo.GuildWarFinalResults GWFR WITH (NOLOCK)
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (GWFR.GuildID = G.GuildID)
			 WHERE GWFR.GuildWarScheduleID = @insGuildWarScheduleID
			 ORDER BY GWFR.MatchSequence;
		END
		ELSE
		BEGIN
			SELECT G.GuildID                -- ?? ID
				 , G.GuildName              -- ???
				 , GWFR.MatchSequence       -- ?? ??
				 , GWFR.MatchTypeCode       -- 0=???, 1=??, 2=4?, 3=8?, 4=16?
				 , GWFR.MatchResultCode     -- 0=???, 1=??, 2=??, 3=??
				 , RewardSkill
				 , RewardItem
				 , RewardMail
				 , RewardPointByOpening
				 , RewardPointByFinal
			  FROM dbo.GuildWarFinalResults GWFR WITH (NOLOCK)
				   INNER JOIN dbo.Guilds G WITH (NOLOCK) ON (GWFR.GuildID = G.GuildID)
			       LEFT OUTER JOIN dbo.GuildWarRewardForGuild R WITH (NOLOCK) ON (G.GuildID = R.GuildID AND GWFR.GuildWarScheduleID = R.GuildWarScheduleID)
			 WHERE GWFR.GuildWarScheduleID = @insGuildWarScheduleID
			 ORDER BY GWFR.MatchSequence;
		END

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
