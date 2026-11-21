USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPageTeacherCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-26
description : ??? ?? ??? ?? ??? ?????? ?????.
result set : CharacterID/bigint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint, GenderCode/tinyint, RespectPoint/int, SelfIntroduction/nvarchar, GraduateCount/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103205 = ??? ??? ????.
history :
4 = ?? ?? ?? ?????. at 2010-07-29
5 = JobCode ?? ?? at 2011-02-17
**/

CREATE PROCEDURE [dbo].[P_GetPageTeacherCharacter]
	  @intStartNumber int --// ?? ??.
	, @intEndNumber int --// ? ??.
	, @inyJobCode tinyint = NULL --// NULL ??? ??. 1=???, 2=??, 3=????, 4=???, 5=????, 10=???, 11=?????, 12=????, 13=???, 14=?????, 15=????, 16=??, 17=??????, 18=????, 19=??, 20=???, 21=??, 22=????, 23=??????, 24=???, 25=????, 26=??????, 27=?????, 28=?????, 29=????, 30=????, 31=????, 32=????, 33=???, 34=?????, 35=????, 36=?????, 37=???, 38=????, 39=?????, 40=??????, 41=???, 42=?????, 43=???, 44=????, 45=?????, 46=????, 47=????, 48=?????, 49=?????, 50=???, 51=???
	, @inyGenderCode tinyint = NULL --// NULL ??? ??. 1=??, 2=??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@nvcSelectStmt nvarchar(max),
	@nvcSelectStmt2 nvarchar(max),
	@inbNullBitmap bigint,
	@intColumnCount int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ?? ??? ?????. (SELECT ?).*/
	SET @nvcSelectStmt = N'TeacherCharacterID, JobCode, GenderCode, RespectPoint, SelfIntroduction, GraduateCount';
	SET @nvcSelectStmt2 = N'CH.CharacterID, CH.CharacterName, CS.CharacterLevel, C1.JobCode, C1.GenderCode, C1.RespectPoint, C1.SelfIntroduction, C1.GraduateCount';
	SET @intColumnCount = 3;
	SET @inbNullBitmap = 
		CASE WHEN @inyJobCode IS NOT NULL THEN POWER(2, 0) ELSE 0 END + 
		CASE WHEN @inyGenderCode IS NOT NULL THEN POWER(2, 1) ELSE 0 END +
		POWER(2, 2);

	/**_# ?? ??? ?????. (FROM, WHERE ?).*/
	SET @nvcStmt = N'
WITH PagingCTE AS (
	SELECT ROW_NUMBER() OVER(ORDER BY RespectPoint DESC) AS RowNumber, ' + @nvcSelectStmt + N'
	FROM dbo.TeacherCharacters WITH (NOLOCK)
'		+ dbo.FN_DynamicWhereClause2(N'JobCode', N'@inyJobCode', @inbNullBitmap, 1, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'GenderCode', N'@inyGenderCode', @inbNullBitmap, 2, @intColumnCount, '=')
		+ dbo.FN_DynamicWhereClause2(N'TeacherListShowFlag', N'CAST(1 AS bit)', @inbNullBitmap, 3, @intColumnCount, '=') + N'
		AND ConnectFlag = CAST(1 AS bit)
)
SELECT ' + @nvcSelectStmt2 + N'
FROM PagingCTE C1
	INNER JOIN dbo.Characters CH WITH (NOLOCK) ON C1.TeacherCharacterID = CH.CharacterID
	INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON C1.TeacherCharacterID = CS.CharacterID
WHERE C1.RowNumber BETWEEN @intStartNumber AND @intEndNumber
ORDER BY C1.RowNumber
OPTION (MAXDOP 1)';

	/**_# ?? ??? ?????.*/
	EXEC dbo.sp_executesql @nvcStmt
		, N'
			  @inyJobCode tinyint
			, @inyGenderCode tinyint
			, @intStartNumber int
			, @intEndNumber int
		'
		, @inyJobCode = @inyJobCode
		, @inyGenderCode = @inyGenderCode
		, @intStartNumber = @intStartNumber
		, @intEndNumber = @intEndNumber;

	/**_## ??? ???? ?? ???? ???, 103205? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103205;
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
