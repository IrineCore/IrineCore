USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPVPGhoulScores]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-07-15

description :
	PVP ?? ?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	3 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_AddPVPGhoulScores]
	  @inbCharacterID	bigint		--// ???ID
	, @intGhoulWin		int = 0		--// ?? ??
	, @intHumanWin		int = 0		--// ?? ??
	, @intTimeOver		int = 0		--// ?? ?? (?? ??)
	, @intGhoulKill		int = 0		--// ?? Kill
	, @intHumanKill		int = 0		--// ?? Kill
	, @intHolyWaterUse	int = 0		--// ?? ??
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

	/**_# [PVPGhoulScores] ???? UPSERT???.*/
	UPDATE dbo.PVPGhoulScores
	   SET PlayCount = PlayCount + 1
		 , GhoulWin = GhoulWin + @intGhoulWin
		 , HumanWin = HumanWin + @intHumanWin
		 , TimeOver = TimeOver + @intTimeOver
		 , GhoulKill = GhoulKill + @intGhoulKill
		 , HumanKill = HumanKill + @intHumanKill
		 , HolyWaterUse = HolyWaterUse + @intHolyWaterUse
		 , UpdateDate = GETDATE()
	 WHERE CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.PVPGhoulScores (CharacterID, PlayCount, GhoulWin, HumanWin, TimeOver, GhoulKill, HumanKill, HolyWaterUse, UpdateDate)
		VALUES (@inbCharacterID, 1, @intGhoulWin, @intHumanWin, @intTimeOver, @intGhoulKill, @intHumanKill, @intHolyWaterUse, GETDATE());
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
