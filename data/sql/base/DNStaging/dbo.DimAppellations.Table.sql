USE [dnstaging]
GO
/****** Object:  Table [dbo].[DimAppellations]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DimAppellations](
	[AppellationID] [int] NOT NULL,
	[AppellationName] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
 CONSTRAINT [PK_DimAppellations] PRIMARY KEY CLUSTERED 
(
	[AppellationID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, FILLFACTOR = 100) ON [PRIMARY]
) ON [PRIMARY]
GO
