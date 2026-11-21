USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuildMaxLevelRewardInfo]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-09-07
description : ???? ?? ?? ?? ??? ?????.
result set : GuildMaxLevelRewardFlag/bit, CharacterJoinLevel/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetGuildMaxLevelRewardInfo]
	  @inbCharacterID bigint --// ??? ID	
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
	/**_# [CharacterStatus] ???? SELECT???.*/
	SELECT ISNULL(CS.GuildMaxLevelRewardFlag,0) AS GuildMaxLevelRewardFlag, ISNULL(GM.CharacterJoinLevel,0) AS CharacterJoinLevel
	FROM dbo.CharacterStatus CS WITH (NOLOCK) 
		 LEFT OUTER JOIN dbo.GuildMembers GM WITH (NOLOCK)
         ON CS.CharacterID = GM.CharacterID		 
	WHERE CS.CharacterID = @inbCharacterID;

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
