USE [dnworld]
GO
/****** Object:  UserDefinedFunction [dbo].[FN_AchieveMissionBin]    Script Date: 11/14/2026 8:54:15 AM ******/
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

CREATE FUNCTION [dbo].[FN_AchieveMissionBin] (
	@inbCharacterID bigint --// ??? ID
)
RETURNS binary(625)
AS
BEGIN
	DECLARE @binMissionBin binary(625), @vchTemp varchar(max) = '';

	WITH C1 (a, b) AS (
		SELECT MissionID / 8, POWER(2, 7 - (MissionID % 8))
		FROM dbo.AchieveMissions
		WHERE CharacterID = @inbCharacterID
	), C2 (a, b) AS (
		SELECT a, CONVERT(char(2), CAST(SUM(b) AS binary(1)), 2)
		FROM C1
		GROUP BY a
	)
	SELECT @vchTemp += COALESCE(C2.b, '00')
	FROM dbo.Tally T
		LEFT OUTER JOIN C2 ON T.n - 1 = C2.a
	WHERE T.n <= 625
	ORDER BY T.n;

	SET @binMissionBin = CONVERT(binary(625), @vchTemp, 2);

	RETURN @binMissionBin;
END
GO
