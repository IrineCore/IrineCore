USE [dnworld]
GO
/****** Object:  Table [dbo].[MapStayLogs2]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MapStayLogs2](
	[ExitDate] [datetime2](0) NOT NULL,
	[MapID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[EntranceDate] [datetime2](0) NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_MapStayLogs2_ExitDate]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE CLUSTERED INDEX [IX_NC_MapStayLogs2_ExitDate] ON [dbo].[MapStayLogs2]
(
	[ExitDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
