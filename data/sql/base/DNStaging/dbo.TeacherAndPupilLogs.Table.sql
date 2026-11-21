USE [dnstaging]
GO
/****** Object:  Table [dbo].[TeacherAndPupilLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TeacherAndPupilLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[TeacherCharacterID] [bigint] NOT NULL,
	[PupilCharacterID] [bigint] NOT NULL,
	[TeacherCharacterJobCode] [tinyint] NOT NULL,
	[TeacherCharacterLevel] [tinyint] NOT NULL,
	[PupilCharacterJobCode] [tinyint] NOT NULL,
	[PupilCharacterLevel] [tinyint] NOT NULL,
	[FavorPoint] [int] NOT NULL,
	[NewTeacherBlockPeriod] [smallint] NOT NULL,
	[PenaltyRespectPoint] [int] NOT NULL,
	[TeacherAndPupilLogCode] [tinyint] NOT NULL
) ON [PRIMARY]
GO
