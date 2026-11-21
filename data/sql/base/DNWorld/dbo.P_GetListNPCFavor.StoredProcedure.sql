USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListNPCFavor]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-14
description : ???? ?? NPC? ???? ?????.
result set : NPCID/int, FavorPoint/smallint, MalicePoint/smallint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = MapID? ???? ??? ?? ?? ??? ?? ?? NPC ??? ????? ?? at 2010-07-20
**/

CREATE PROCEDURE [dbo].[P_GetListNPCFavor]
	  @inbCharacterID bigint --// ??? ID
	, @intMapID int = NULL --// ? ID
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
	IF @intMapID IS NULL
		SELECT NPCID, FavorPoint, MalicePoint
		FROM dbo.NPCFavor WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID
		ORDER BY NPCID;

	ELSE
		SELECT NF.NPCID, NF.FavorPoint, NF.MalicePoint
		FROM dbo.NPCFavor NF WITH (NOLOCK)
			INNER JOIN dbo.NPCLocation NL WITH (NOLOCK) ON NF.NPCID = NL.NPCID
		WHERE NL.MapID = @intMapID AND NF.CharacterID = @inbCharacterID
		ORDER BY NF.NPCID;
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
