USE [dnworld]
GO
/****** Object:  Table [dbo].[TeacherAndPupilLogs0]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TeacherAndPupilLogs0](
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
/****** Object:  Index [IX_NC_TeacherAndPupilLogs0_LogDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_TeacherAndPupilLogs0_LogDate] ON [dbo].[TeacherAndPupilLogs0]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
