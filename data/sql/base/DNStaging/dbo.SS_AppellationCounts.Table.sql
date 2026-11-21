USE [dnstaging]
GO
/****** Object:  Table [dbo].[SS_AppellationCounts]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SS_AppellationCounts](
	[CharacterID] [bigint] NOT NULL,
	[AppellationCount] [smallint] NOT NULL,
 CONSTRAINT [PK_SS_AppellationCounts] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
