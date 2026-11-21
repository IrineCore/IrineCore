USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGuildWarFinalGuildMembers]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-09-16

description :
	??? ?? ?? ??? ??? ??
result set : 
	GuildID				int
	CharacterID			bigint
	CharacterClassCode	tinyint
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	3 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListGuildWarFinalGuildMembers]
	  @intGuildID int --// ?? ID
	, @insGuildWarScheduleID smallint --// ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dtBasisDate smalldatetime;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SELECT @dtBasisDate = RewardEndDate
	  FROM dbo.GuildWarSchedule WITH (NOLOCK)
	 WHERE GuildWarScheduleID = @insGuildWarScheduleID;

	SELECT GM.GuildID, C.CharacterID, C.CharacterClassCode
	  FROM dbo.GuildMembers GM WITH (NOLOCK)
		   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (GM.CharacterID = C.CharacterID)
	 WHERE GM.GuildID = @intGuildID
	   AND GM.RegisterDate < @dtBasisDate
	   AND C.DeleteFlag = 0;

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

SET ANSI_NULLS ON;
GO
