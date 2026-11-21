USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModNPCLocation]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-14
description : ?? ?? ?? ?, NPC ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ?? ?? ???? 1?? ???? ???? ?? at 2010-07-16
3 = ?? ?? ???? ?? at 2010-07-16
4 = ?? ?? at 2010-07-16
6 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_ModNPCLocation]
	  @vchNPCIDs varchar(max) --// ,? ?????? NPC ID ???
	, @vchMapIDs varchar(max) --// ,? ?????? NPC? ??? ? ID ???
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
	BEGIN TRANSACTION;

	TRUNCATE TABLE dbo.NPCLocation;

	INSERT dbo.NPCLocation (MapID, NPCID)
	SELECT A.string, B.string
	FROM dbo.TF_Split(@vchMapIDs, ',') A
		INNER JOIN dbo.TF_Split(@vchNPCIDs, ',') B ON A.seq = B.seq
	WHERE A.string > '' AND B.string > ''
	OPTION (KEEPFIXED PLAN);

	COMMIT TRANSACTION;
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
