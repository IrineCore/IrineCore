USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_LogoutCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 27
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-12
description : ??? ??? ?????. (??? ????? ?? ?? ?? ????)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
11 = TeacherCharacters.ConnectFlag ? 0?? UPDATE??? ?? at 2010-07-27
12 = MapStayLog ??? ?? ? ?? ??? ?? ??? ??? ???? ?? ?? ????? ?? at 2010-08-24
13 = ?????? LastSessionID? 0?? ??????? ?? at 2011-01-24
14 = ??? ???? ????? ?? at 2011-03-08
16 = ??? ?? ?? at 2011-03-18
17 = ??? ??? ?? ?? at 2011-03-29
18 = input param. @intViewCashEquipmentBitmap ?? at 2011-05-18
19 = ??? ???? ???? ??? ?? at 2011-06-21
22 = input param. @binCheckSumBin? ???? ??? ?? (?? ???? ?? ??) at 2013-04-16
24~25 = input param. @bitPCRoomFlag ??  at 2013-05-30 by ???
26 = input param. @binCheckSumBin? ??  at 2013-06-03 by ???
**/

CREATE PROCEDURE [dbo].[P_LogoutCharacter]
	  @inbCharacterID bigint --// ??? ID
	, @intSessionID int --// ?? ID
	, @intViewCashEquipmentBitmap int = NULL --// ??? ??? ?? (?? or ??) ???
	, @bitPCRoomFlag bit = NULL --// 0=??, 1=PC?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intLastSessionID int
	, @intLastLoginIP int
	, @inyJobCode tinyint
	, @intLastMapID int
	, @dt2LastLoginDate datetime2(0)
	, @inyCharacterLevel tinyint
	, @dt2LastMapEntranceDate datetime2(0);

DECLARE @nvcAccountName nvarchar(50)
	, @nvcCharacterName nvarchar(30)
	, @nvcGuildName nvarchar(50)
	, @intWorldID int
	, @nvcStmt nvarchar(4000);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	UPDATE dbo.CharacterStatus
	SET @inyCharacterLevel = CharacterLevel
		, @inyJobCode = JobCode
		, @intLastMapID = LastMapID
		, @dt2LastLoginDate = LastLoginDate
		, @intLastSessionID = LastSessionID
		, @intLastLoginIP = LastLoginIP
		, @dt2LastMapEntranceDate = LastMapEntranceDate
		, LastLogoutDate = @dt2Now
		, LastSessionID = 0
		, ViewCashEquipmentBitmap = ISNULL(@intViewCashEquipmentBitmap, ViewCashEquipmentBitmap)
	WHERE CharacterID = @inbCharacterID;

	UPDATE dbo.TeacherCharacters
	SET ConnectFlag = 0
	WHERE TeacherCharacterID = @inbCharacterID;
	IF @intSessionID = @intLastSessionID AND @dt2LastLoginDate < @dt2Now
		EXEC @intReturnValue = dbo.P_AddCharacterLogoutLog
			  @dt2LogoutDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @dt2LoginDate = @dt2LastLoginDate
			, @intLoginIP = @intLastLoginIP
			, @intSessionID = @intSessionID
			, @bitPCRoomFlag = @bitPCRoomFlag;

	SET @dt2LastMapEntranceDate = CASE WHEN @dt2LastLoginDate > @dt2LastMapEntranceDate THEN @dt2LastLoginDate ELSE @dt2LastMapEntranceDate END;

	IF @intSessionID = @intLastSessionID AND @dt2LastMapEntranceDate < @dt2Now
		EXEC @intReturnValue = dbo.P_AddMapStayLog
			  @dt2ExitDate = @dt2Now
			, @intMapID = @intLastMapID
			, @inbCharacterID = @inbCharacterID
			, @inyJobCode = @inyJobCode
			, @inyCharacterLevel = @inyCharacterLevel
			, @dt2EntranceDate = @dt2LastMapEntranceDate;
	
	IF @bitPCRoomFlag = 1 AND @intSessionID = @intLastSessionID AND @dt2LastLoginDate < @dt2Now
		EXEC @intReturnValue = dbo.P_AddPCRoomCharacterLogoutLog
			  @dt2LogoutDate = @dt2Now
			, @inbCharacterID = @inbCharacterID
			, @dt2LoginDate = @dt2LastLoginDate
			, @intLoginIP = @intLastLoginIP;

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
