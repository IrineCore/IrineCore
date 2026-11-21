USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPVPGhoulScores]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-07-15

description :
	PVP ?? ?? ?? ??
result set : 
	@intPlayCount		int
	@intGhoulWin		int
	@intHumanWin		int
	@intTimeOver		int
	@intGhoulKill		int
	@intHumanKill		int
	@intHolyWaterUse	int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetPVPGhoulScores]
	  @inbCharacterID	bigint		--// ???ID
	, @intPlayCount		int OUTPUT	--// ?? ?? ??? ??
	, @intGhoulWin		int OUTPUT	--// ?? ??
	, @intHumanWin		int OUTPUT	--// ?? ??
	, @intTimeOver		int OUTPUT	--// ?? ?? (?? ??)
	, @intGhoulKill		int OUTPUT	--// ?? Kill
	, @intHumanKill		int OUTPUT	--// ?? Kill
	, @intHolyWaterUse	int OUTPUT	--// ?? ??
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

	/**_# ??? ???? 1?? ?? SELECT ???.*/
	SELECT @intPlayCount = PlayCount
		 , @intGhoulWin = GhoulWin
		 , @intHumanWin = HumanWin
		 , @intTimeOver = TimeOver
		 , @intGhoulKill = GhoulKill
		 , @intHumanKill = HumanKill
		 , @intHolyWaterUse = HolyWaterUse
	  FROM dbo.PVPGhoulScores WITH (NOLOCK)
	 WHERE CharacterID = @inbCharacterID;

	/**_## ??? ???? ?? ???? ??? 0 ?(?) ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SELECT @intPlayCount = 0
			 , @intGhoulWin = 0
			 , @intHumanWin = 0
			 , @intTimeOver = 0
			 , @intGhoulKill = 0
			 , @intHumanKill = 0
			 , @intHolyWaterUse = 0
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
