USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPlayAlteia]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-10-22
description : ???? ??? ??? ?????.
result set :
	CharacterID/bigint
  , AcquireGoldKeyCount/int
  , GoldKeyRegisterDate/datetime2(0)
  , PlayTime/int
  , PlayTimeRegisterDate/datetime2(0)
  , DailyPlayCount/int
  , WeeklyPlayCount/int
  , LastPlayingDate/datetime2(0)
  , GuildID/int
  , GuildAcquireGoldKeyCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103214 = ?? ??? ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_GetPlayAlteia]
	  @inbCharacterID bigint --// ???ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2Today datetime2(0) = CAST(GETDATE() AS date);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	
	SELECT A.CharacterID
		 , A.AcquireGoldKeyCount
		 , A.GoldKeyRegisterDate
		 , A.PlayTime
		 , A.PlayTimeRegisterDate
		 , A.DailyPlayCount
		 , A.WeeklyPlayCount
		 , A.LastPlayingDate
		 , B.GuildID
	     , C.AcquireGoldKeyCount AS GuildAcquireGoldKeyCount
	FROM dbo.PlayAlteia A WITH(NOLOCK, INDEX(PK_PlayAlteia), FORCESEEK)
		LEFT OUTER join dbo.GuildMembers B WITH(NOLOCK, INDEX(IX_UN_GuildMembers_CharacterID), FORCESEEK) ON A.CharacterID = B.CharacterID
		LEFT OUTER JOIN dbo.AlteiaGuildRank C WITH(NOLOCK, INDEX(PK_AlteiaGuildRank), FORCESEEK) ON B.GuildID = C.GuildID 
	WHERE A.CharacterID = @inbCharacterID	
	OPTION(KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103214;
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
