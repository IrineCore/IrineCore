USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetListBlockedDNGirl]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-02
description : ??? ??? ???? ?????.
result set : DNGirlCharacterID/bigint, CharacterName/nvarchar, JobCode/tinyint, CharacterLevel/tinyint, BlockedReason/nvarchar, BlockedDate/datetime
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_GetListBlockedDNGirl]	  
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
	/**_# [BlockedDNGirls] ???? SELECT???.*/
	SELECT BG.DNGirlCharacterID, CH.CharacterName, CS.JobCode, CS.CharacterLevel, BG.BlockedReason, BG.BlockedDate
	FROM dbo.BlockedDNGirls BG WITH (NOLOCK) INNER JOIN dbo.Characters CH WITH (NOLOCK) ON BG.DNGirlCharacterID = CH.CharacterID
		INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID;
END TRY
BEGIN CATCH	

	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
