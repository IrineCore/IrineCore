USE [dnstaging]
GO
/****** Object:  Table [dbo].[CUCountLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CUCountLogs](
	[LogDate] [datetime] NOT NULL,
	[WorldID] [int] NOT NULL,
	[CUCount] [int] NOT NULL,
	[ChannelPartnerCode] [tinyint] NULL
) ON [PRIMARY]
GO
