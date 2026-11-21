USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCharacterCount]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCharacterCount]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCharacterCount] AS' 
END
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-19
description : ?? ? ?? ??? ?? ?????.
result set : ??? ???? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101143 = ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_GetCharacterCount]
	  @nvcAccountName nvarchar(50) --// ??? ??
	, @intDeleteWaitingTime int = 10080 --// ?? ?? ??. ?? : minute
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblTemp table (
	WorldID int NOT NULL PRIMARY KEY,
	CharacterCount int NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblTemp (WorldID, CharacterCount)
	SELECT WorldID, COUNT(*) AS CharacterCount
	FROM dbo.Accounts ACC WITH (NOLOCK, INDEX(IX_UN_Accounts_AccountName), FORCESEEK)
		INNER JOIN dbo.Characters CH WITH (NOLOCK, INDEX(IX_NN_Characters_AccountID), FORCESEEK) ON ACC.AccountID = CH.AccountID
		INNER JOIN dbo.DeletedCharacters DCH WITH (NOLOCK, INDEX(PK_DeletedCharacters), FORCESEEK) ON CH.CharacterID = DCH.CharacterID
	WHERE ACC.AccountName = @nvcAccountName
		AND CH.CharacterName IS NOT NULL
		AND CH.DeleteByAdminFlag = CAST(0 AS bit)
		AND DCH.DeleteDate > DATEADD(MINUTE, -@intDeleteWaitingTime, GETDATE())
	GROUP BY WorldID
	OPTION (KEEPFIXED PLAN, MAXDOP 1);

	WITH C2 AS (
		SELECT V.WorldID, CAST(V.CharacterCount AS int) AS CharacterCount
		FROM dbo.Accounts ACC WITH (NOLOCK)
			INNER JOIN dbo.VIX_CharacterCount V WITH (NOEXPAND, NOLOCK) ON ACC.AccountID = V.AccountID
		WHERE ACC.AccountName = @nvcAccountName
	)
	SELECT COALESCE(C1.WorldID, C2.WorldID) AS WorldID, ISNULL(C1.CharacterCount, 0) + ISNULL(C2.CharacterCount, 0) AS CharacterCount
	FROM @tblTemp C1
		FULL OUTER JOIN C2 ON C1.WorldID = C2.WorldID
	ORDER BY WorldID
	OPTION (MAXDOP 1);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101143
		GOTO ErrorHandler;
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
