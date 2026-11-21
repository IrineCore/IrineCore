USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModNPCFavor]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-14
description : NPC ??/?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = MERGE???? UPSERT???? ?? at 2010-08-19
3 = @vchFavorPoints, @vchMalicePoint ? ???? 0??? ? INSERT??? ?? at 2010-08-19
5 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_ModNPCFavor]
	  @inbCharacterID bigint --// ??? ID
	, @vchNPCIDs varchar(8000) --// ,? ?????? NPC ID ???
	, @vchFavorPoints varchar(8000) --// ,? ?????? NPC ? ?? ??? ???
	, @vchMalicePoint varchar(8000) --// ,? ?????? NPC ? ?? ??? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE @tblSource table (
	NPCID int NOT NULL PRIMARY KEY,
	FavorPoint smallint NOT NULL,
	MalicePoint smallint NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblSource (NPCID, FavorPoint, MalicePoint)
	SELECT A.string, B.string, C.string
	FROM dbo.TF_Split(@vchNPCIDs, ',') A
		INNER JOIN dbo.TF_Split(@vchFavorPoints, ',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchMalicePoint, ',') C ON A.seq = C.seq
	WHERE A.string > '' AND B.string > '' AND C.string > ''
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

	/**_# [NPCFavor] ???? MERGE???.*/
	WITH T AS (
		SELECT CharacterID, NPCID, FavorPoint, MalicePoint
		FROM dbo.NPCFavor WITH (UPDLOCK)
		WHERE CharacterID = @inbCharacterID
	)
	MERGE T
	USING @tblSource AS S
	ON T.CharacterID = @inbCharacterID AND T.NPCID = S.NPCID
	WHEN MATCHED AND (S.FavorPoint > 0 OR S.MalicePoint > 0) THEN
		UPDATE SET FavorPoint = S.FavorPoint, MalicePoint = S.MalicePoint
	WHEN MATCHED AND S.FavorPoint = 0 AND S.MalicePoint = 0 THEN
		DELETE
	WHEN NOT MATCHED BY TARGET AND (S.FavorPoint > 0 OR S.MalicePoint > 0) THEN
		INSERT (CharacterID, NPCID, FavorPoint, MalicePoint)
		VALUES (@inbCharacterID, S.NPCID, S.FavorPoint, S.MalicePoint)
	OPTION (KEEPFIXED PLAN, LOOP JOIN);

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
