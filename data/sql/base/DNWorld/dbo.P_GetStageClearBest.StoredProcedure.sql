USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetStageClearBest]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-12-12
description : ?? ?? ???? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetStageClearBest]
	  @intMapID int --// ?ID
	, @inbLegendCharacterID bigint OUTPUT --// ??? ?? ???ID
	, @nvcLegendCharacterName nvarchar(50) OUTPUT --// ??? ?? ?????
	, @inyLegendClearGradeCode tinyint OUTPUT --// ??? ?? ????(10=SSS, 11=SS, 12=S, 13=A, 14=B, 15=C, 16=D)
	, @intLegendClearPoint int OUTPUT --// ??? ?? CP
	, @inbMonthlyCharacterID bigint OUTPUT --// ?? ???ID
	, @nvcMonthlyCharacterName nvarchar(50) OUTPUT --// ?? ?????
	, @inyMonthlyClearGradeCode tinyint OUTPUT --// ?? ????(10=SSS, 11=SS, 12=S, 13=A, 14=B, 15=C, 16=D)
	, @intMonthlyClearPoint int OUTPUT --//  ?? ?? CP
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
	/**_# [StageClearBest] ???? ?????.*/
	SELECT @inbLegendCharacterID = CharacterID
         , @nvcLegendCharacterName = CharacterName
         , @inyLegendClearGradeCode = ClearGradeCode
         , @intLegendClearPoint = ClearPoint
	FROM dbo.StageClearBest WITH(NOLOCK, INDEX(PK_StageClearBest), FORCESEEK)
	WHERE MapID = @intMapID
	OPTION(KEEPFIXED PLAN);

	/**_# [StageClearMonthlyBest] ???? ?????.*/
	SELECT @inbMonthlyCharacterID = CharacterID
         , @nvcMonthlyCharacterName = CharacterName
         , @inyMonthlyClearGradeCode = ClearGradeCode
         , @intMonthlyClearPoint = ClearPoint
	FROM dbo.StageClearMonthlyBest WITH(NOLOCK, INDEX(PK_StageClearMonthlyBest), FORCESEEK)
	WHERE MapID = @intMapID
	OPTION(KEEPFIXED PLAN);

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
