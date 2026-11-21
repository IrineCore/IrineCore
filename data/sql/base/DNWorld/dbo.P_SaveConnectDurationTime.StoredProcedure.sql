USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_SaveConnectDurationTime]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-02-11
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5 = CheckFlag ?? ???? ?? ?? ?? at 2011-03-10
9 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
**/

CREATE PROCEDURE [dbo].[P_SaveConnectDurationTime]
	  @inbCharacterID bigint --// ??? ID
	, @vchConnectDurationKeys varchar(8000) --// ,? ?????? ?? ?? ??? Key ? ???.
	, @vchDurationTimes varchar(8000) --// ,? ?????? ?? ?? ?? ??? (?? : ms)
	, @vchCheckFlags varchar(8000) = '' --// ,? ?????? bit ?? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblConnectDurations table (
	CharacterID bigint NOT NULL,
	ConnectDurationKey int NOT NULL PRIMARY KEY,
	DurationTime bigint NOT NULL,
	CheckFlag bit NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblConnectDurations (CharacterID, ConnectDurationKey, DurationTime, CheckFlag)
	SELECT @inbCharacterID, A.string, B.string, ISNULL(C.string, 0)
	FROM dbo.TF_Split(@vchConnectDurationKeys, ',') A
		INNER JOIN dbo.TF_Split(@vchDurationTimes, ',') B ON A.seq = B.seq
		LEFT OUTER JOIN dbo.TF_Split(@vchCheckFlags, ',') C ON A.seq = C.seq
	WHERE A.string > '' AND B.string > ''
	OPTION (FORCE ORDER, KEEPFIXED PLAN);

	WITH T AS (
		SELECT CharacterID, ConnectDurationKey, DurationTime, CheckFlag
		FROM dbo.ConnectDurations
		WHERE CharacterID = @inbCharacterID
	)
	MERGE T
	USING @tblConnectDurations S
	ON T.CharacterID = S.CharacterID AND T.ConnectDurationKey = S.ConnectDurationKey
	WHEN MATCHED THEN
		UPDATE SET DurationTime = S.DurationTime, CheckFlag = S.CheckFlag
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (CharacterID, ConnectDurationKey, DurationTime, CheckFlag)
		VALUES (S.CharacterID, S.ConnectDurationKey, S.DurationTime, S.CheckFlag)
	WHEN NOT MATCHED BY SOURCE THEN
		DELETE
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
