USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListVIPCharacter]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListVIPCharacter]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListVIPCharacter] AS' 
END
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-07
description : ?? ?? ?? ??? ???? VIP ???? ?????.
result set : AccountID, AccountName, WorldID, CharacterID, CharacterName, DeleteFlag, VIPPoint, RemainVIPDays, AutoPayFlag, VIPEndDate
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101117 = ???? ???? ????.
101166 = VIP ?? ??? ????.
history :
2 = ?? ??? VIP ?? ?? ?? at 2010-07-08
**/

ALTER PROCEDURE [dbo].[P_GetListVIPCharacter]
	  @nvcAccountName nvarchar(50) = NULL --// ??? ?? ??
	, @nvcCharacterName nvarchar(30) = NULL --// ??? ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @i int = 1
	, @j int
	, @inbCharacterID bigint
	, @intVIPTotalPoint int
	, @dtsVIPStartDate smalldatetime
	, @dtsVIPEndDate smalldatetime
	, @bitAutoPayFlag bit;

DECLARE @tblTemp table (Seq int IDENTITY(1, 1) PRIMARY KEY, CharacterID bigint NOT NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ?? ?? ??? ???? ??? ??*/
	IF @nvcAccountName IS NOT NULL
		INSERT @tblTemp (CharacterID)
		SELECT CH.CharacterID
		FROM dbo.Accounts AC WITH (NOLOCK)
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON AC.AccountID = CH.AccountID
		WHERE AC.AccountName = @nvcAccountName;

	ELSE
		INSERT @tblTemp (CharacterID)
		SELECT CharacterID
		FROM dbo.Characters WITH (NOLOCK)
		WHERE CharacterName = @nvcCharacterName;

	/**_# ???? ???? ??? 101117? ?????.*/
	SET @j = @@ROWCOUNT;

	IF @j = 0
	BEGIN
		SET @intReturnValue = 101117;
		GOTO ErrorHandler;
	END

	/**_# ? ???? VIP ???? ?????.*/
	WHILE @i <= @j
	BEGIN
		SELECT @inbCharacterID = CharacterID FROM @tblTemp WHERE Seq = @i;

		EXEC dbo.P_GetVIPPoint
			  @inbCharacterID = @inbCharacterID
			, @intVIPTotalPoint = @intVIPTotalPoint OUTPUT
			, @dtsVIPStartDate = @dtsVIPStartDate OUTPUT
			, @dtsVIPEndDate = @dtsVIPEndDate OUTPUT
			, @bitAutoPayFlag = @bitAutoPayFlag OUTPUT;

		SET @i += 1;
	END

	SELECT ACC.AccountID, ACC.AccountName, CH.WorldID, CH.CharacterID, CH.CharacterName, CH.DeleteFlag
		, (V.BasicPoint + V.IncrementalPoint + V.AdjustedPoint - V.DecreasedPoint) AS VIPPoint
		, CASE WHEN DATEDIFF(DAY, GETDATE(), V.VIPEndDate) < 0 THEN 0 ELSE DATEDIFF(DAY, GETDATE(), V.VIPEndDate) + 1 END AS RemainVIPDays
		, V.AutoPayFlag
		, V.VIPEndDate
	FROM @tblTemp T
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON T.CharacterID = CH.CharacterID
		INNER JOIN dbo.Accounts ACC WITH (NOLOCK) ON CH.AccountID = ACC.AccountID
		INNER JOIN dbo.VIPPoints V WITH (NOLOCK) ON T.CharacterID = V.CharacterID
	ORDER BY CH.DeleteFlag DESC, CH.CharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101166;
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
