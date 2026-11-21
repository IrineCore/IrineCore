USE [dnworld]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_AppellationBin]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-26
description : ??? ??? bitmap ???? ?????.
**/

CREATE FUNCTION [dbo].[FN_AppellationBin] (
	@inbCharacterID bigint --// ??? ID
)
RETURNS binary(100)
AS
BEGIN
	DECLARE @binAppellationBin binary(100), @vchTemp varchar(max) = '';

	WITH C1 (a, b) AS (
		SELECT AppellationID / 8, POWER(2, 7 - (AppellationID % 8))
		FROM dbo.Appellations
		WHERE CharacterID = @inbCharacterID
	), C2 (a, b) AS (
		SELECT a, CONVERT(char(2), CAST(SUM(b) AS binary(1)), 2)
		FROM C1
		GROUP BY a
	)
	SELECT @vchTemp += COALESCE(C2.b, '00')
	FROM dbo.Tally T
		LEFT OUTER JOIN C2 ON T.n - 1 = C2.a
	WHERE T.n <= 100
	ORDER BY T.n;

	SET @binAppellationBin = CONVERT(binary(100), @vchTemp, 2);

	RETURN @binAppellationBin;
END
GO
