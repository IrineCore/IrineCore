USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPVPTournamentResult]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-10-26
description : PVP ???? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddPVPTournamentResult]
	  @inyPlayerCount tinyint --// ?? ?? ?
	, @vchCharacterIDs varchar(1000) --// ???ID ex)4??? ?? ???  123,345,678,901
	, @vchFinalRound varchar(1000) --// ??? ?? ex)4??? ?? ??? 2,4,2,4
	, @vchResultFlag varchar(1000) --// 0=??, 1=?? ex)4??? ?? ??? 1,0,0,0
	, @inbGambleID bigint = NULL --// (OPTIONAL) ??? ?? GambleID ??!
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
      , @inbPVPTournamentID bigint;
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END
BEGIN TRY
	IF @inbGambleID > 0
		SELECT @dt2Now = GambelStartDate
		FROM dbo.GambleIDs WITH(NOLOCK)
		WHERE GambleID = @inbGambleID

	/**_# [PVPTournamentIDs] ???? INSERT???.*/
	INSERT dbo.PVPTournamentIDs (PVPTournamentResultDate, PlayerCount, GambleID)
	VALUES (@dt2Now, @inyPlayerCount, @inbGambleID);

	SET @inbPVPTournamentID = SCOPE_IDENTITY();

	/**_# [PVPTournamentResults] ???? INSERT???.*/
	INSERT dbo.PVPTournamentResults (PVPTournamentID, PVPTournamentSeqence, CharacterID, FinalRound, ResultFlag)
	SELECT @inbPVPTournamentID, A.seq, CAST(A.string AS bigint), CAST(B.string AS tinyint), CAST(C.string AS bit)
	FROM dbo.TF_Split(@vchCharacterIDs, ',') A 
		INNER JOIN dbo.TF_Split(@vchFinalRound, ',') B ON A.seq = B.seq
		INNER JOIN dbo.TF_Split(@vchResultFlag, ',') C ON B.seq = C.seq;

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
