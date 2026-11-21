USE [dnworld]
GO
/****** Object:  Table [dbo].[WorldCoverage]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[WorldCoverage](
	[PublisherCode] [tinyint] NOT NULL,
	[PartitionID] [tinyint] NOT NULL,
	[WorldID] [int] NOT NULL,
 CONSTRAINT [PK_WorldCoverage] PRIMARY KEY CLUSTERED 
(
	[PublisherCode] ASC,
	[PartitionID] ASC,
	[WorldID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
INSERT [dbo].[WorldCoverage] ([PublisherCode], [PartitionID], [WorldID]) VALUES (1, 1, 1)
GO
