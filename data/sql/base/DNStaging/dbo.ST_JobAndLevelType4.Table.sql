USE [dnstaging]
GO
/****** Object:  Table [dbo].[ST_JobAndLevelType4]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ST_JobAndLevelType4](
	[BasisDate] [date] NOT NULL,
	[WorldID] [int] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[CharacterCount] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_ST_JobAndLevelType4_BasisDate]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_ST_JobAndLevelType4_BasisDate] ON [dbo].[ST_JobAndLevelType4]
(
	[BasisDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
