USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarOpeningPoint]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-06-11

description :
	??? ?? ?? ?? (?? ???? ??? ???? ?? ??)
result set : 
	CharacterID / bigint or GuildID / int or TeamColorCode / tinyint
	OpeningPoint / int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	1~3 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarOpeningPoint]
      @chrType char(1) --// C=??? ?? ??? ??, G=?? ????? ??, T=? ????? ??
    , @inbID bigint = null --// @chrType=C ?? ??? ID, @chrType=G ?? ?? ID, @chrType=T ?? (1=??, 2=??), null ?? ?? ??
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

    /**_# @bitType=C ?? ??? ?? ???? ?????. */
    IF @chrType = 'C'
    BEGIN
		IF @inbID IS NOT NULL
		BEGIN
			SELECT CharacterID, OpeningPoint
			  FROM dbo.VIX_GuildWarOpeningPointForCharacter WITH (NOEXPAND, NOLOCK)
			 WHERE CharacterID = @inbID;
		END
		ELSE
		BEGIN
			SELECT CharacterID, OpeningPoint
			  FROM dbo.VIX_GuildWarOpeningPointForCharacter WITH (NOEXPAND, NOLOCK);
		END
    END
    /**_# @bitType=G ?? ?? ?? ???? ?????. */
    ELSE IF @chrType = 'G'
    BEGIN
		IF @inbID IS NOT NULL
		BEGIN
			SELECT GuildID, OpeningPoint
			  FROM dbo.VIX_GuildWarOpeningPointForGuild WITH (NOEXPAND, NOLOCK)
			 WHERE GuildID = CONVERT(int, @inbID);
		END
		ELSE
		BEGIN
			SELECT GuildID, OpeningPoint
			  FROM dbo.VIX_GuildWarOpeningPointForGuild WITH (NOEXPAND, NOLOCK);
		END
    END
    /**_# @bitType=T ?? ? ?? ???? ?????. */
    ELSE IF @chrType = 'T'
    BEGIN
		IF @inbID IS NOT NULL
		BEGIN
			SELECT TeamColorCode, OpeningPoint
			  FROM dbo.VIX_GuildWarOpeningPointForTeam WITH (NOEXPAND, NOLOCK)
			 WHERE TeamColorCode = CONVERT(tinyint, @inbID);
		END
		ELSE
		BEGIN
			SELECT TeamColorCode, OpeningPoint
			  FROM dbo.VIX_GuildWarOpeningPointForTeam WITH (NOEXPAND, NOLOCK);
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
