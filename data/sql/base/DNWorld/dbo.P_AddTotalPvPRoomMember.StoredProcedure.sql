USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddTotalPvPRoomMember]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-09-12
description : ???? PVP?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103368 = ????? ?????? ???? ??? ? ????.
103373 = ????? PvP?? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_AddTotalPvPRoomMember]
	  @intTotalPvPRoomID int --// ???? PVP? ID
	, @inbCharacterID bigint --// ??? ID
	, @bitObserverFlag bit --// 0=??, 1=???
	, @inyMaxPlayerCount tinyint --// ?? ?? ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @inyPvPRoomMemberCount tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION
	
	/**_# [TotalPvPRoomMembers] ???? INSERT???.*/
	INSERT dbo.TotalPvPRoomMembers (TotalPvPRoomID, CharacterID, ObserverFlag)
	VALUES (@intTotalPvPRoomID, @inbCharacterID, @bitObserverFlag);

	/**_# ?? PVP?? ???? ?????.*/
	SELECT @inyPvPRoomMemberCount = PvPRoomMemberCount 
	FROM dbo.VIX_TotalPvPRoomMemberCount WITH(NOEXPAND, NOLOCK)
	WHERE TotalPvPRoomID = @intTotalPvPRoomID;

	/**_# ?? ???? ??????? ?? 103368 ?????.*/
	IF @inyPvPRoomMemberCount > @inyMaxPlayerCount
	BEGIN
		SET @intReturnValue = 103368;
		GOTO ErrorHandler;
	END
		
	COMMIT TRANSACTION
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_TotalPvPRooms_TotalPvPRoomMembers_TotalPvPRoomID%'
		SET @intReturnValue = 103373;

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
