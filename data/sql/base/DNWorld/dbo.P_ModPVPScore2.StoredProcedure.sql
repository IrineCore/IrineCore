USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModPVPScore2]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-03-07
description : PVP ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
1 = ????
**/

CREATE PROCEDURE [dbo].[P_ModPVPScore2]
	  @inbCharacterID bigint --// ??? ID
	, @intPVPWin int --// PVP ?
	, @intPVPLose int --// PVP ?
	, @intPVPDraw int --// PVP ???
	, @intPVPGiveUp int --// PVP ??
	, @inyPVPLevel tinyint --// PVP ??
	, @intPVPExp int --// PVP ???
	, @intPVPPlayTime int --// PVP ??? ?? (?)
	, @inbPVPKillPoint bigint --// PVP ? ???
	, @inbPVPAssisPoint bigint --// PVP ???? ???
	, @inbPVPSupportPoint bigint --// PVP ??? ???
	, @vchCharacterClassCode varchar(100) = '' --// ,? ?????? CharacterClassCode ???
    , @vchWin varchar(100) = '' --// ,? ?????? ?? ???
    , @vchLose varchar(100) = '' --// ,? ?????? ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblResults table (
	CharacterClassCode tinyint NOT NULL PRIMARY KEY,
	Win int NOT NULL,
	Lose int NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	
	INSERT @tblResults (CharacterClassCode, Win, Lose)
	SELECT A.string, B.string, C.string
	FROM dbo.TF_Split(@vchCharacterClassCode, ',') A
		INNER JOIN dbo.TF_Split(@vchWin, ',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchLose, ',') C ON A.seq = C.seq
	WHERE A.string > '' AND B.string > '' AND C.string > ''
	OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

	/**_# [PVPScores]???? UPDATE???.*/
	UPDATE dbo.PVPScores
	SET PVPWin = @intPVPWin
		, PVPLose = @intPVPLose
		, PVPDraw = @intPVPDraw
		, PVPGiveUp = @intPVPGiveUp
		, PVPLevel = @inyPVPLevel
		, PVPExp = @intPVPExp
		, PVPPlayTime = @intPVPPlayTime
		, PVPKillPoint = @inbPVPKillPoint
		, PVPAssisPoint = @inbPVPAssisPoint
		, PVPSupportPoint = @inbPVPSupportPoint
	WHERE CharacterID = @inbCharacterID;

	/**_## UPDATE? ?? ??? [PVPScores]???? INSERT???.*/
	IF @@ROWCOUNT = 0
	BEGIN
	    INSERT dbo.PVPScores (CharacterID, PVPWin, PVPLose, PVPDraw, PVPGiveUp, PVPLevel, PVPExp, PVPPlayTime, PVPKillPoint, PVPAssisPoint, PVPSupportPoint)
	    VALUES (@inbCharacterID, @intPVPWin, @intPVPLose, @intPVPDraw, @intPVPGiveUp, @inyPVPLevel, @intPVPExp, @intPVPPlayTime, @inbPVPKillPoint, @inbPVPAssisPoint, @inbPVPSupportPoint)
	END
	
	;WITH T AS (
		SELECT CharacterID, CharacterClassCode, Win, Lose
		FROM dbo.PVPCharacterClassScores WITH(NOLOCK)
		WHERE CharacterID = @inbCharacterID	
	), S (CharacterClassCode, Win, Lose) AS(
		SELECT CharacterClassCode, Win, Lose 
		FROM @tblResults
	)
	MERGE T
	USING S
	ON T.CharacterClassCode = S.CharacterClassCode
	WHEN MATCHED THEN
		UPDATE SET Win = S.Win, Lose = S.Lose
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (CharacterID, CharacterClassCode, Win, Lose)
		VALUES (@inbCharacterID, S.CharacterClassCode, Win, Lose);

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
