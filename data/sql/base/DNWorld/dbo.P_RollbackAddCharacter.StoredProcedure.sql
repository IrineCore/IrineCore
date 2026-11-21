USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_RollbackAddCharacter]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-01
description : ???? ??? ?????. ?? : ? SP??? ???? DNMembership DB? dbo.P_RollbackAddCharacter? ???? ??? ?? ??? ???? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = dbo.AppellationBinaries ???? ? ?? ???? ?? at 2010-07-26
3 = dbo.CompleteQuestBinaires ???? ? ?? ???? ?? at 2010-08-30
5 = [ItemLocation] ??? ??? ?? ?? by ??? at 2011-12-20
6 = [PVPScores] ??? ???? ?? ?? by ??? at 2012-03-07
**/

CREATE PROCEDURE [dbo].[P_RollbackAddCharacter]
	  @inbCharacterID bigint --// ??? ??? ??? ID
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
	/**_# P_FirstUseCharacter? ?? INSERT? ? ??*/
	DELETE dbo.Skills WHERE CharacterID = @inbCharacterID;
	DELETE dbo.QuickSlots WHERE CharacterID = @inbCharacterID;
	DELETE dbo.Notifiers WHERE CharacterID = @inbCharacterID;
	DELETE dbo.Quests WHERE CharacterID = @inbCharacterID;
	DELETE dbo.PVPScores WHERE CharacterID = @inbCharacterID;
	DELETE dbo.PVPCharacterClassScores WHERE CharacterID = @inbCharacterID;
	DELETE dbo.MissionBinaries WHERE CharacterID = @inbCharacterID;

	/**_# P_AddMaterializedItem? ?? INSERT? ? ??*/
	DELETE dbo.MaterializedItems WHERE OwnerCharacterID = @inbCharacterID;

	/**_# P_AddCharacter? ?? INSERT? ? ??*/
	DELETE dbo.JobChangeLogs WHERE CharacterID = @inbCharacterID;
	DELETE dbo.CharacterStatus WHERE CharacterID = @inbCharacterID;
	DELETE dbo.Characters WHERE CharacterID = @inbCharacterID;
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
